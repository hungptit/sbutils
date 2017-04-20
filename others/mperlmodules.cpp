#include "boost/program_options.hpp"
#include <array>
#include <fstream>
#include <iostream>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

#include "sbutils/FileSearch.hpp"
#include "sbutils/FileUtils.hpp"
#include "sbutils/FolderDiff.hpp"
#include "sbutils/Resources.hpp"
#include "sbutils/Timer.hpp"
#include "sbutils/UtilsTBB.hpp"

namespace {
    struct GetPerlModule {
      public:
        using output_type = std::string;
        GetPerlModule(const std::string &pref, const std::string &suff)
            : Prefix(pref), Suffix(suff){};
        output_type operator()(const std::string &aPath) {
            auto const begin = aPath.cbegin() + Prefix.size() + 1;
            auto const end = aPath.cend() - Suffix.size();
            std::string result;
            result.reserve(std::distance(begin, end) + 8);

            // Replace / by ::
            for (auto it = begin; it != end; ++it) {
                if (*it == '/') {
                    result.push_back(':');
                    result.push_back(':');
                } else {
                    result.push_back(*it);
                }
            }

            return result;
        }

      private:
        std::string Prefix;
        std::string Suffix;
    };

    template <typename Container> void print(Container &&results, const std::string &sep) {
        fmt::MemoryWriter writer;
        std::for_each(results.begin(), results.end(),
                      [&writer, &sep](auto const &item) { writer << (item) << sep; });
        fmt::print("{}", writer.str());
    }

    template <typename Container, typename Op, typename FirstConstraint,
              typename... Constraints>
    auto filter_tbb2(Container &&data, Op &&op, FirstConstraint &&f1, Constraints &&... fs) {
        // utils::ElapsedTime<MILLISECOND> t1("Filtering files: ");
        // using input_type = typename Container::value_type;
        using output_type = typename std::decay<Op>::type::output_type;
        tbb::concurrent_vector<output_type> results;

        auto filterObj = [&op, &f1, &fs..., &results, &data](const int idx) {
            auto const &item = data[idx];
            if (isValid(item, f1, std::forward<Constraints>(fs)...)) {
                results.push_back(op(item.Path));
            }
        };

        int size = static_cast<int>(data.size());
        tbb::parallel_for(0, size, 1, filterObj);
        return results;
    }
}

int main(int argc, char *argv[]) {
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    std::string database;
    std::vector<std::string> extensions{".pm"};
    std::vector<std::string> folders;
    std::string pattern;

    // clang-format off
    desc.add_options()
        ("help,h", "Print this help")
        ("verbose,v", "Display searched data.")
        ("folders,f", po::value<std::vector<std::string>>(&folders), "Search folders.")
        ("pattern,p", po::value<std::string>(&pattern)->default_value("_Test.pm"), "Module pattern.")
        ("database,d", po::value<std::string>(&database)->default_value(sbutils::Resources::Database), "File database.");
    // clang-format on

    po::positional_options_description p;
    p.add("folders", -1);
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << "Usage: mperlmodules [options]\n";
        std::cout << desc;
        std::cout << "Examples:\n";
        std::cout << "\t mperlmodules -d .database -s prod/perllib/Athena/WorkUnit\n";
        return EXIT_SUCCESS;
    }

    bool verbose = vm.count("verbose");

    // Print verbose information
    if (verbose) {
        fmt::print("Database: {}\n", database);
        fmt::print("Pattern: {}\n", pattern);
    }

    sbutils::ElapsedTime<sbutils::MILLISECOND> timer("Total time: ", verbose);

    if (verbose) {
        std::cout << "Database: " << database << std::endl;
    }

    using Container = std::vector<sbutils::FileInfo>;
    std::sort(folders.begin(), folders.end());
    auto data = sbutils::read_baseline<Container>(database, folders, verbose);
    const sbutils::ExtFilter<std::vector<std::string>> f1(extensions);
    const sbutils::SimpleFilter f2(pattern);

    GetPerlModule op("prod/perlib/Athena/", "_Test.pm");
    auto results = filter_tbb2(data, op, f1, f2);
    print(results, " ");
}
