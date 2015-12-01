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

#include "utils/BFSFileSearch.hpp"
#include "utils/DFSFileSearch.hpp"
#include "utils/FileSearch.hpp"
#include "utils/FolderDiff.hpp"
#include "utils/LevelDBIO.hpp"
#include "utils/Timer.hpp"
#include "utils/Utils.hpp"

#include <sstream>
#include <string>
#include <vector>

namespace {
    class ElapsedTime {
      public:
        ~ElapsedTime() {
            std::cout << "Elapsed time: "
                      << Timer_.toc() / Timer_.ticksPerSecond() << " seconds"
                      << std::endl;
        }

      private:
        Timer Timer_;
    };

    // TODO: Need to add filter to this function.
    template <typename Container> void print(Container &data, bool verbose) {
        if (verbose) {
            for (auto item : data) {
                std::cout << item << "\n";
            }
        } else {
            for (auto item : data) {
                fmt::print("{}\n", std::get<0>(item));
            }
        }
    }

    auto diff(Utils::Reader &reader, std::string &aPath, bool verbose) {
        boost::filesystem::path p(aPath); // Use relative path
        auto aKey = p.string();
        if (verbose) {
            std::cout << "Current path: " << p << "\n";
            std::cout << "Current key: " << aKey << "\n";
        }

        Timer timer;

        typedef Utils::FileSearchBase<Utils::BFSFileSearchBase> FileSearch;
        FileSearch finder;
        typedef std::vector<Utils::FileInfo> Container;
        Utils::FolderDiff<Container> diff;

        diff.find(finder, aKey);
        if (verbose) {
            std::cout << "Find time: " << timer.toc() / timer.ticksPerSecond()
                      << " seconds" << std::endl;
        }

        timer.tic();
        auto dict = diff.read(reader, aKey);
        if (verbose) {
            std::cout << "Read time: " << timer.toc() / timer.ticksPerSecond()
                      << " seconds" << std::endl;
        }

        timer.tic();

        auto results = diff.diff(finder.getData(), dict);
        if (verbose) {
            std::cout << "Diff time: " << timer.toc() / timer.ticksPerSecond()
                      << " seconds" << std::endl;
        }
        return results;
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
    ("parallel,p", "Use threaded version of viewer.")
    ("keys,k", "List all keys.")
    ("folders,f", po::value<std::vector<std::string>>(), "Search folders.")
    ("stems,s", po::value<std::vector<std::string>>(), "File stems.")
    ("extensions,e", po::value<std::vector<std::string>>(), "File extensions.")
    ("strings,t", po::value<std::vector<std::string>>(), "Search string")
    ("database,d", po::value<std::string>(), "File database.");
    // clang-format on

    po::positional_options_description p;
    p.add("database", -1);
    po::variables_map vm;
    po::store(
        po::command_line_parser(argc, argv).options(desc).positional(p).run(),
        vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << "Usage: viewer [options]\n";
        std::cout << desc;
        std::cout << "Examples:" << std::endl;
        std::cout << "\t viewer -d .database -s AutoFix" << std::endl;
        return 0;
    }

    bool verbose = false;
    if (vm.count("verbose")) {
        verbose = true;
    }

    bool isThreaded = false;
    if (vm.count("parallel")) {
        isThreaded = true;
    }

    // Search folders
    std::vector<std::string> folders;
    if (vm.count("folders")) {
        folders = vm["folders"].as<std::vector<std::string>>();
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
        dataFile = (boost::filesystem::path(Utils::FileDatabaseInfo::Database))
                       .string();
    }

    if (verbose) {
        std::cout << "Database: " << dataFile << std::endl;
    }

    Utils::Reader reader(dataFile);

    {
        ElapsedTime e;

        // Find files
        for (auto aPath : folders) {
            std::vector<Utils::FileInfo> editedFiles, newFiles, deletedFiles;
            std::tie(editedFiles, newFiles, deletedFiles) =
                diff(reader, aPath, verbose);

            // Now we will display the results
            std::cout << "---- Modified files: " << editedFiles.size()
                      << " ----\n";
            print(editedFiles, verbose);

            std::cout << "---- New files: " << newFiles.size() << " ----\n";
            Utils::print(newFiles);

            std::cout << "---- Deleted files: " << deletedFiles.size()
                      << " ----\n";
            Utils::print(deletedFiles);
        }
    }
}