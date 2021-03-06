// This is an independent project of an individual developer. Dear PVS-Studio,
// please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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

#include "sbutils/DataStructures.hpp"
#include "sbutils/FileSearch.hpp"
#include "sbutils/FolderDiff.hpp"
#include "sbutils/Timer.hpp"

#include <sstream>
#include <string>
#include <vector>

#include "tbb/task_scheduler_init.h"

namespace {
    struct NormalFilter {
      public:
        bool isValid(sbutils::FileInfo &item) {
            return (std::find(ExcludedExtensions.begin(), ExcludedExtensions.end(),
                              item.Extension) == ExcludedExtensions.end());
        }

      private:
        std::vector<std::string> ExcludedExtensions = {};
    };

    template <typename Container, typename Filter>
    void print(Container &data, Filter &f, const std::string &prefix) {
        for (auto item : data) {
            if (f.isValid(item)) {
                fmt::print("{0}{1}\n", prefix, item.Path);
            }
        }
    }
} // namespace

int main(int argc, char *argv[]) {
    using namespace boost;

    namespace po = boost::program_options;
    po::options_description desc("Allowed options");

    std::string dataFile;
    std::vector<std::string> folders;
    unsigned int numberOfThreads;

    // clang-format off
    desc.add_options()
        ("help,h", "Print this help")
        ("verbose,v", "Display searched data.")
		("max-threads", po::value<unsigned int>(&numberOfThreads)->default_value(2), "Specify the maximum number of used threads.")
        ("folders,f", po::value<std::vector<std::string>>(&folders), "Search folders.")
        ("database,d", po::value<std::string>(&dataFile)->default_value(sbutils::Resources::Database), "File information database.");
    // clang-format on

    po::positional_options_description p;
    p.add("folders", -1);
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc;
        fmt::print("Example:\n\tmdiff prod/\n");
        return 0;
    }

    if (folders.empty()) {
        fmt::print("You must provide folder paths!\n");
        return EXIT_FAILURE;
    }

    bool verbose = vm.count("verbose");
    if (verbose) {
        std::cout << "Database: " << dataFile << std::endl;
    }

    {
        tbb::task_scheduler_init task_scheduler(numberOfThreads);
        sbutils::ElapsedTime<sbutils::SECOND> e("Diff time: ", verbose);
        std::vector<sbutils::FileInfo> allEditedFiles, allNewFiles, allDeletedFiles;

        std::tie(allEditedFiles, allDeletedFiles, allNewFiles) =
            sbutils::diffFolders_tbb(dataFile, folders, verbose);

        // Now we will display the results
        NormalFilter f;
        print(allEditedFiles, f, "*");
        print(allNewFiles, f, "+");
        print(allDeletedFiles, f, "-");
    }
}
