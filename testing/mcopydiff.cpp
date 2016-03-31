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
    struct DonothingFilter {
      public:
        bool isValid(utils::FileInfo &) const { return true; }

      private:
        std::vector<std::string> ExcludedExtensions = {".p"};
    };

    struct NormalFilter {
      public:
        bool isValid(utils::FileInfo &item) const {
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

    void copyFiles(const std::vector<utils::FileInfo> &files,
                   const boost::filesystem::path &dstDir,
                   bool verbose = false) {
        using namespace boost::filesystem;
        boost::system::error_code errcode;
        auto options = copy_option::overwrite_if_exists;
        for (auto item : files) {
            auto srcFile = path(std::get<0>(item));
            auto dstFile = dstDir / srcFile;
            bool needCopy = true;
            if (exists(dstFile)) {
                // We do not copy a given file if the source and destination
                // have the same sizes.
                needCopy &= (file_size(srcFile) != file_size(dstFile));
                if (needCopy) {
                    permissions(dstFile, add_perms | owner_write);
                }
            } else {
                auto parent = dstFile.parent_path();
                if (!exists(parent)) {
                    create_directories(parent);
                    if (verbose)
                    std::cout << "Create " << parent << "\n";
                }
            }

            if (needCopy) {
                copy_file(srcFile, dstFile, options);
                if (verbose) {
                    fmt::print("Copy {0} to {1}\n", srcFile.string(),
                               dstFile.string());
                }
            }
        }
    }

    void deleteFiles(const std::vector<utils::FileInfo> &files,
                     const boost::filesystem::path &parent,
                     bool verbose = false) {
        using namespace boost::filesystem;
        boost::system::error_code errcode;
        for (auto const &item : files) {
            auto aFile = path(std::get<0>(item));
            auto dstFile = parent / aFile;
            if (exists(dstFile)) {
                permissions(dstFile, add_perms | owner_write);
                remove(dstFile);
                if (verbose) {
                    fmt::print("Delete {0}\n", dstFile.string());
                }
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
    ("src_dir,s", po::value<std::string>(), "Source sandbox.")
    ("dst_dir,d", po::value<std::string>(), "Destination sandbox.")
      ("baseline,b", po::value<std::string>(), "File database.");
    // clang-format on

    po::positional_options_description p;
    p.add("baseline", -1);
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

    std::string srcDir;
    if (vm.count("src_dir")) {
        srcDir = utils::normalize_path(vm["src_dir"].as<std::string>());
    }

    std::string dstDir;
    if (vm.count("dst_dir")) {
        dstDir = utils::normalize_path(vm["dst_dir"].as<std::string>());
    }

    // Get file database
    std::string dataFile;
    if (vm.count("baseline")) {
        dataFile = utils::normalize_path(vm["baseline"].as<std::string>());
    } else {
        dataFile = utils::Resources::Database;
    }

    if (verbose) {
        std::cout << "Database: " << dataFile << std::endl;
    }

    {
        utils::ElapsedTime<utils::SECOND> e;
        std::vector<utils::FileInfo> allEditedFiles, allNewFiles,
            allDeletedFiles;
        std::tie(allEditedFiles, allDeletedFiles, allNewFiles) =
            utils::diffFolders(dataFile, {srcDir}, verbose);

        // We will copy new files and edited files to the destiation
        // folder. We also remove all deleted files in the destination
        // folder.
        fmt::print("Copying {} edited files\n", allEditedFiles.size());
        copyFiles(allEditedFiles, dstDir, verbose);

        fmt::print("Copying {} new files\n", allNewFiles.size());
        copyFiles(allNewFiles, dstDir, verbose);

        fmt::print("Deleting {} files\n", allDeletedFiles.size());
        deleteFiles(allDeletedFiles, dstDir, verbose);
    }
}
