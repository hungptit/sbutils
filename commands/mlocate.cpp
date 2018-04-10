// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "boost/program_options.hpp"
#include <array>
#include <string>
#include <tuple>
#include <vector>

#include "CommandUtils.hpp"
#include "FileSearch.hpp"
#include "FileUtils.hpp"
#include "FolderDiff.hpp"
#include "Resources.hpp"
#include "Timer.hpp"
#include "UtilsTBB.hpp"

#include "tbb/task_scheduler_init.h"
#include "fmt/format.h"

template <typename Container> void print(Container &&results) {
    fmt::MemoryWriter writer;
    std::for_each(results.begin(), results.end(),
                  [&writer](auto const &item) { writer << item.Path << "\n"; });
    fmt::print("{}", writer.str());
}

int main(int argc, char *argv[]) {
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    sbutils::MLocateArgs args;
    unsigned int numberOfThreads;

    // clang-format off
    desc.add_options()
        ("help,h", "Print this help")
        ("verbose,v", "Display searched data.")
		("max-threads", po::value<unsigned int>(&numberOfThreads)->default_value(1), "Specify the maximum number of used threads.")
        ("folders,f", po::value<std::vector<std::string>>(&args.Folders), "Search folders.")
        ("stems,s", po::value<std::vector<std::string>>(&args.Stems), "File stems.")
        ("extensions,e", po::value<std::vector<std::string>>(&args.Extensions), "File extensions.")
        ("pattern,p", po::value<std::string>(&args.Pattern), "Search string pattern.")
        ("database,d", po::value<std::string>(&args.Database)->default_value(sbutils::Resources::Database), "File database.");
    // clang-format on

    po::positional_options_description p;
    p.add("pattern", -1);
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << "Usage: mlocate [options]\n";
        std::cout << desc;
        std::cout << "Examples:\n";
        std::cout << "\t mlocate -d .database -s AutoFix\n";
        std::cout << "\t mlocate -s AutoFix # if the current folder contains a file "
                     "information database i.e \".database\" folder\n";
        return 0;
    }

    // Check that the given database is valid
    {
        boost::filesystem::path aPath(args.Database);
        if (!boost::filesystem::exists(args.Database)) {
            throw std::runtime_error("File information database \"" + args.Database +
                                     "\" does not exist\n");
        }
    }

    args.Verbose = vm.count("verbose");
    sbutils::ElapsedTime<sbutils::MILLISECOND> timer("Total time: ", args.Verbose);
    tbb::task_scheduler_init task_scheduler(numberOfThreads);

    if (args.Verbose) {
        std::cout << "Database: " << args.Database << std::endl;
    }

    // Display files that match given constraints.
    print(sbutils::LocateFiles(args));
}
