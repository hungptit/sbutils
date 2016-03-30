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

#define BOOST_THREAD_VERSION 4
#include "boost/config.hpp"
#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"
#include "boost/thread.hpp"
#include "boost/thread.hpp"
#include "boost/thread/future.hpp"
#include "boost/unordered_set.hpp"

#include "utils/FileSearch.hpp"
#include "utils/FolderDiff.hpp"
#include "utils/LevelDBIO.hpp"
#include "utils/Timer.hpp"

#include "cppformat/format.h"

#include <sstream>
#include <string>
#include <vector>

namespace {
    struct NormalFilter {
      public:
        bool isValid(utils::FileInfo &item) {
            return (std::find(ExcludedExtensions.begin(),
                              ExcludedExtensions.end(),
                              std::get<utils::filesystem::EXTENSION>(item)) ==
                    ExcludedExtensions.end());
        }

      private:
        std::vector<std::string> ExcludedExtensions = {".p"};
    };

    template <typename Container, typename Filter>
    void print(Container &data, Filter &f) {
        for (auto item : data) {
            if (f.isValid(item)) {
                fmt::print("{}\n", std::get<utils::filesystem::PATH>(item));
            }
        }
    }
}


int main(int argc, char *argv[]) {
    using namespace boost;
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");

    // clang-format off
  desc.add_options()
    ("help,h", "Print this help")
    ("verbose,v", "Display searched data.")
    ("keys,k", "List all keys.")
    ("source_sandbox,s", po::value<std::string>(), "Source sandbox.")
    ("dest_sandbox,t", po::value<std::string>(), "Destination sandbox.")
    ("database,d", po::value<std::string>(), "File database.");
    // clang-format on

    po::positional_options_description p;
    p.add("folders", -1);
    po::variables_map vm;
    po::store(
        po::command_line_parser(argc, argv).options(desc).positional(p).run(),
        vm);
    po::notify(vm);

    if (vm.count("help")) {
        // std::cout << "Usage:  [options]\n";
        std::cout << desc;
        std::cout << "Examples:" << std::endl;
        std::cout << "\t viewer matlab/src matlab/test" << std::endl;
        return 0;
    }

    bool verbose = false;
    if (vm.count("verbose")) {
        verbose = true;
    }

    // Search folders
    std::string sourceSandbox;
    if (vm.count("source_sandbox")) {
      folders = vm["source_sandbox"].as<std::string>();
    }

    std::string destSandbox;
    if (vm.count("dest_sandbox")) {
      folders = vm["dest_sandbox"].as<std::string>();
    }

    // Get file stems
    std::vector<std::string> stems;
    if (vm.count("stems")) {
        stems = vm["stems"].as<std::vector<std::string>>();
    }

    // Get file extensions
    std::vector<std::string> extensions;
    if (vm.count("extensions")) {
        extensions = vm["extensions"].as<std::vector<std::string>>();
    }

    // Get file extensions
    std::vector<std::string> searchStrings;
    if (vm.count("strings")) {
        searchStrings = vm["strings"].as<std::vector<std::string>>();
    }

    // Get file database
    std::string dataFile;
    if (vm.count("database")) {
        dataFile = vm["database"].as<std::string>();
    } else {
        dataFile =
            (boost::filesystem::path(utils::Resources::Database)).string();
    }

    if (verbose) {
        std::cout << "Database: " << dataFile << std::endl;
    }

    {
        utils::ElapsedTime<utils::SECOND> e;
        std::vector<utils::FileInfo> allEditedFiles, allNewFiles,
            allDeletedFiles;
        std::tie(allEditedFiles, allDeletedFiles, allNewFiles) =
            utils::diffFolders(dataFile, folders, verbose);

        // Now we will display the results
        std::cout << "---- Modified files: " << allEditedFiles.size()
                  << " ----\n";
        NormalFilter f;
        print(allEditedFiles, f);

        std::cout << "---- New files: " << allNewFiles.size() << " ----\n";
        print(allNewFiles, f);

        std::cout << "---- Deleted files: " << allDeletedFiles.size()
                  << " ----\n";
        print(allDeletedFiles, f);
    }
}
