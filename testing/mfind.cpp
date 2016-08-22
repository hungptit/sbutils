#include <array>
#include <fstream>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"

#include "fmt/format.h"
#include "utils/FileSearch.hpp"
#include "utils/Print.hpp"
#include "utils/Timer.hpp"
#include "utils/Utils.hpp"

int main(int argc, char *argv[]) {
    using namespace boost;
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    utils::Timer timer;

    // clang-format off
    desc.add_options()
        ("help,h", "Print this help")
        ("verbose,v", "Display searched data.")
        ("toJSON,j", po::value<std::string>(), "Output results in a JSON file.")
        ("folders,f", po::value<std::vector<std::string>>(), "Search folders.")
        ("file-stems,s", po::value<std::vector<std::string>>(), "File stems.")
        ("extensions,e", po::value<std::vector<std::string>>(), "File extensions.")
        ("pattern,p", po::value<std::string>(), "A search pattern.");
    // clang-format on

    po::positional_options_description p;
    p.add("folders", -1);
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc;
        std::cout << "Examples:" << std::endl;
        std::cout << "\t mfind ./ -d .database" << std::endl;
        return EXIT_SUCCESS;
    }

    auto verbose = false;
    if (vm.count("verbose")) {
        verbose = true;
    }

    std::string jsonFile;
    if (vm.count("toJSON")) {
        jsonFile = vm["toJSON"].as<std::string>();
    }

    using boost::filesystem::path;

    boost::system::error_code errcode;
    std::vector<std::string> folders;
    if (vm.count("folders")) {
        for (auto item : vm["folders"].as<std::vector<std::string>>()) {
            folders.emplace_back(path(item).string());
        }
    }

    std::vector<std::string> stems;
    if (vm.count("file-stems")) {
        stems = vm["file-stems"].as<std::vector<std::string>>();
    }

    std::vector<std::string> extensions;
    if (vm.count("extensions")) {
        extensions = vm["extensions"].as<std::vector<std::string>>();
    }

    std::string pattern;
    if (vm.count("pattern")) {
        pattern = vm["pattern"].as<std::string>();
    }

    // Search for files in the given folders.
    using path = boost::filesystem::path;
    using Container = std::vector<path>;
    utils::filesystem::SimpleVisitor<Container, utils::filesystem::NormalPolicy> visitor;
    Container searchFolders;
    for (auto item : folders) {
        searchFolders.emplace_back(path(item));
    }
    utils::filesystem::dfs_file_search(searchFolders, visitor);
    auto const & results = visitor.getResults();
    const utils::ExtFilter<std::vector<std::string>> f1(extensions);
    const utils::StemFilter<std::vector<std::string>> f2(stems);
    const utils::SimpleFilter f3(pattern);
    auto data =
        (pattern.empty()) ? utils::filter(results, f1, f2) : utils::filter(results, f1, f2, f3);

    if (verbose) {
        fmt::print("Search folders:\n");
        for (const auto &val : folders) {
            fmt::print("{}\n", val);
        }
        fmt::print("Number of files: {}\n", data.size());
        std::for_each(data.begin(), data.end(), [](auto const &val) {
            fmt::print("({0}, {1}, {2}, {3})\n", val.Path, val.Size, val.Permissions,
                       val.TimeStamp);
        });
    } else {
        fmt::print("Number of files: {}\n", data.size());
        std::for_each(data.begin(), data.end(),
                      [](auto const &val) { fmt::print("{0}\n", val.Path); });
    }

    if (!jsonFile.empty()) {
        std::ostringstream os;
        {
            cereal::JSONOutputArchive oar(os);
            oar("Search results", results);
        }

        // Write to a JSON file
        std::ofstream myfile(jsonFile);
        myfile << os.str() << std::endl;
    }

    std::cout << "Total time: " << timer.toc() / timer.ticksPerSecond() << " seconds"
              << std::endl;
    return 0;
}
