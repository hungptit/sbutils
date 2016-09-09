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
#include "utils/FileSearch.hpp"
#include "utils/FolderDiff.hpp"
#include "utils/LevelDBIO.hpp"
#include "utils/Timer.hpp"

#include <future>
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
            return (std::find(ExcludedExtensions.begin(), ExcludedExtensions.end(),
                              item.Extension) == ExcludedExtensions.end());
        }

      private:
        std::vector<std::string> ExcludedExtensions = {".p"};
    };

    template <typename Container, typename Filter> void print(Container &data, Filter &f) {
        for (auto item : data) {
            if (f.isValid(item)) {
                fmt::print("{}\n", item.Path);
            }
        }
    }

    void createParentFolders(const boost::filesystem::path &dstDir,
                             const std::vector<utils::FileInfo> &files, bool verbose = false) {
        auto createParentObj = [&dstDir, verbose](const utils::FileInfo &info) {
            using namespace boost::filesystem;
            path aFile(dstDir / path(info.Path));
            path parentFolder(aFile.parent_path());
            if (!exists(parentFolder)) {
                create_directories(parentFolder);
                if (verbose) {
                    fmt::print("Create folder: {}\n", parentFolder.string());
                }
            }
        };
    }

    auto copyFiles(const std::vector<utils::FileInfo> &files,
                   const boost::filesystem::path &dstDir, bool verbose = false) {
        using namespace boost::filesystem;
        size_t nfiles = 0, nbytes = 0;
        boost::system::error_code errcode;
        auto options = copy_option::overwrite_if_exists;
        for (auto item : files) {
            auto srcFile = path(item.Path);
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
                nbytes += item.Size;
                if (verbose) {
                    fmt::print("Copy {0} to {1}\n", srcFile.string(), dstFile.string());
                }
            }
        }
        return std::make_tuple(nfiles, nbytes);
    }

    auto deleteFiles(const std::vector<utils::FileInfo> &files,
                     const boost::filesystem::path &parent, bool verbose = false) {
        using namespace boost::filesystem;
        size_t nfiles = 0;
        boost::system::error_code errcode;
        for (auto const &item : files) {
            auto aFile = path(item.Path);
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
        ("src_dir,s", po::value<std::vector<std::string>>(), "Source folder.")
        ("dst_dir,d", po::value<std::vector<std::string>>(), "Destination sandbox.")
        ("baseline,b", po::value<std::string>(), "File database.");
    // clang-format on

    po::positional_options_description p;
    p.add("src_dir", -1);
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc;
        fmt::print("Examples:\n\tmcopydiff matlab/toolbox matlab/test -d "
                   "/sandbox/hungdang/mdlrefadvisor\n");
        return 0;
    }

    bool verbose = false;
    if (vm.count("verbose")) {
        verbose = true;
    }

    std::vector<std::string> srcDir;
    if (vm.count("src_dir")) {
        auto srcPaths = vm["src_dir"].as<std::vector<std::string>>();
        for (auto const &item : srcPaths) {
            srcDir.emplace_back(utils::normalize_path(item));
        }
    } else {
        throw(std::runtime_error("Need to provide the source folder path!"));
    }

    std::vector<std::string> dstDir;
    if (vm.count("dst_dir")) {
        auto inputArgs = vm["dst_dir"].as<std::vector<std::string>>();
        for (auto const &anArg : inputArgs) {
            dstDir.emplace_back(utils::normalize_path(anArg));
        }
    } else {
        throw(std::runtime_error("Need to provide the destination sandbox path!"));
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
        std::vector<utils::FileInfo> allEditedFiles, allNewFiles, allDeletedFiles;
        std::tie(allEditedFiles, allDeletedFiles, allNewFiles) =
            utils::diffFolders(dataFile, srcDir, verbose);

        // We will copy new files and edited files to the destiation
        // folder. We also remove all deleted files in the destination
        // folder.
        utils::ElapsedTime<utils::SECOND> e("Copy files: ");
        for (auto const &aDstDir : dstDir) {
            auto copyEditedFileObj = [&allEditedFiles, &aDstDir, verbose]() {
                return copyFiles(allEditedFiles, aDstDir, verbose);
            };

            auto copyNewFileObj = [&allNewFiles, &aDstDir, verbose]() {
                return copyFiles(allNewFiles, aDstDir, verbose);
            };

            auto deleteFileObj = [&allDeletedFiles, &aDstDir, verbose]() {
                return deleteFiles(allDeletedFiles, aDstDir, verbose);
            };

            using namespace boost;
            future<std::tuple<size_t, size_t>> t1 = async(copyEditedFileObj);
            future<std::tuple<size_t, size_t>> t2 = async(copyNewFileObj);
            future<size_t> t3 = async(deleteFileObj);

            t1.wait();
            t2.wait();
            t3.wait();

            auto results1 = t1.get();
            auto results2 = t2.get();
            auto results3 = t3.get();

            fmt::print("==== Summary for {} ====\n", aDstDir);
            fmt::print("\tCopied {0} modified files ({1} bytes)\n", std::get<0>(results1),
                       std::get<1>(results1));
            fmt::print("\tCopied {0} new files ({1} bytes)\n", std::get<0>(results2),
                       std::get<1>(results2));
            fmt::print("\tDelete {0} files\n", results3);
        }
    }
}
