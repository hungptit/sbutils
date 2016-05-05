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

#include "fmt/format.h"

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

    // TODO: Need to parallelize this function
    auto copyFiles(const std::vector<utils::FileInfo> &files,
                   const boost::filesystem::path &dstDir,
                   bool verbose = false) {
        using namespace boost::filesystem;
        size_t nfiles = 0, nbytes = 0;
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
                nfiles++;
                nbytes += std::get<utils::filesystem::FILESIZE>(item);
                if (verbose) {
                    fmt::print("Copy {0} to {1}\n", srcFile.string(),
                               dstFile.string());
                }
            }
        }
        return std::make_tuple(nfiles, nbytes);
    }

    auto deleteFiles(const std::vector<utils::FileInfo> &files,
                     const boost::filesystem::path &parent,
                     bool verbose = false) {
        using namespace boost::filesystem;
        size_t nfiles = 0;
        boost::system::error_code errcode;
        for (auto const &item : files) {
            auto aFile = path(std::get<0>(item));
            auto dstFile = parent / aFile;
            if (exists(dstFile)) {
                permissions(dstFile, add_perms | owner_write);
                remove(dstFile);
                nfiles++;
                if (verbose) {
                    fmt::print("Delete {0}\n", dstFile.string());
                }
            }
        }
        return nfiles;
    }
}

int main(int argc, char *argv[]) {
    using namespace boost;
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");

    // clang-format off
  desc.add_options()
    ("help,h", "Print this help")
    ("verbose,v", "Display more information.")
    ("keys,k", "List all keys.")
    ("src_dir,s", po::value<std::string>(), "Source folder.")
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
        std::cout << desc;
        fmt::print("Examples:\n\tmcopydiff -s matlab/ -d "
                   "/sandbox/hungdang/mdlrefadvisor\n");
        return 0;
    }

    bool verbose = false;
    if (vm.count("verbose")) {
        verbose = true;
    }

    std::string srcDir;
    if (vm.count("src_dir")) {
        srcDir = utils::normalize_path(vm["src_dir"].as<std::string>());
    } else {
        throw(std::runtime_error("Need to provide the source folder path!"));
    }

    std::string dstDir;
    if (vm.count("dst_dir")) {
        dstDir = utils::normalize_path(vm["dst_dir"].as<std::string>());
    } else {
        throw(std::runtime_error(
            "Need to provide the destination sandbox path!"));
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
        auto copyEditedFileObj = [&allEditedFiles, &dstDir, verbose]() {
            return copyFiles(allEditedFiles, dstDir, verbose);
        };

        auto copyNewFileObj = [&allNewFiles, &dstDir, verbose]() {
            return copyFiles(allNewFiles, dstDir, verbose);
        };

        auto deleteFileObj = [&allDeletedFiles, &dstDir, verbose]() {
            return deleteFiles(allDeletedFiles, dstDir, verbose);
        };

        // auto results1 = copyEditedFileObj();
        // auto results2 = copyNewFileObj();
        // auto results3 = deleteFileObj();

        boost::future<std::tuple<size_t, size_t>> t1 =
            boost::async(copyEditedFileObj);
        boost::future<std::tuple<size_t, size_t>> t2 =
            boost::async(copyNewFileObj);
        ;
        boost::future<size_t> t3 = boost::async(deleteFileObj);
        ;

        t1.wait();
        t2.wait();
        t3.wait();

        auto results1 = t1.get();
        auto results2 = t2.get();
        auto results3 = t3.get();

        fmt::print("Summary:\n");
        fmt::print("\tCopied {0} modified files ({1} bytes)\n",
                   std::get<0>(results1), std::get<1>(results1));
        fmt::print("\tCopied {0} new files ({1} bytes)\n",
                   std::get<0>(results2), std::get<1>(results2));
        fmt::print("\tDelete {0} files\n", results3);
    }
}
