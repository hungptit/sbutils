#include <array>
#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"

#include "fmt/format.h"
#include "utils/DataStructures.hpp"
#include "utils/FileSearch.hpp"
#include "utils/FolderDiff.hpp"
#include "utils/Timer.hpp"
#include <sstream>
#include <string>
#include <vector>

namespace {
    struct NormalFilter {
      public:
        bool isValid(utils::FileInfo &item) {
            return (std::find(ExcludedExtensions.begin(), ExcludedExtensions.end(),
                              item.Extension) == ExcludedExtensions.end());
        }

      private:
        std::vector<std::string> ExcludedExtensions = {".p",      ".d",  ".o",   ".ts",
                                                       ".xml~",   ".m~", ".log", ".dbg",
                                                       ".mexa64", ".so", ".dot", ".tmp"};
    };

    template <typename Container, typename Filter> void print(Container &data, Filter &f) {
        for (auto item : data) {
            if (f.isValid(item)) {
                fmt::print("{}\n", item.Path);
            }
        }
    }
}

int main(int argc, char *argv[]) {
    using namespace boost;
    utils::ElapsedTime<utils::SECOND> e("Diff time: ");

    namespace po = boost::program_options;
    po::options_description desc("Allowed options");

    // clang-format off
  desc.add_options()
    ("help,h", "Print this help")
    ("verbose,v", "Display searched data.")
    ("keys,k", "List all keys.")
    ("folders,f", po::value<std::vector<std::string>>(), "Search folders.")
    ("database,d", po::value<std::string>(), "File database.");
    // clang-format on

    po::positional_options_description p;
    p.add("folders", -1);
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc;
        fmt::print("Example:\n\tmdiff matlab/toolbox\n");
        return 0;
    }

    bool verbose = false;
    if (vm.count("verbose")) {
        verbose = true;
    }

    // Search folders
    std::vector<std::string> folders;
    if (vm.count("folders")) {
        folders = vm["folders"].as<std::vector<std::string>>();
    } else {
        folders = {"matlab/src", "matlab/toolbox", "matlab/test", "matlab/resources"};
    }

    // Get file database
    std::string dataFile;
    if (vm.count("database")) {
        dataFile = vm["database"].as<std::string>();
    } else {
        dataFile = (boost::filesystem::path(utils::Resources::Database)).string();
    }

    if (verbose) {
        std::cout << "Database: " << dataFile << std::endl;
    }

    {
        std::vector<utils::FileInfo> allEditedFiles, allNewFiles, allDeletedFiles;

        std::tie(allEditedFiles, allDeletedFiles, allNewFiles) =
            utils::diffFolders_tbb(dataFile, folders, verbose);

        // Now we will display the results
        std::cout << "---- Modified files: " << allEditedFiles.size() << " ----\n";
        NormalFilter f;
        print(allEditedFiles, f);

        std::cout << "---- New files: " << allNewFiles.size() << " ----\n";
        print(allNewFiles, f);

        std::cout << "---- Deleted files: " << allDeletedFiles.size() << " ----\n";
        print(allDeletedFiles, f);
    }
}
