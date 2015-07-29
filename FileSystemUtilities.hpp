#ifndef FileSystemUtilities_hpp_
#define FileSystemUtilities_hpp_

#include <string>
#include <fstream>
#include <iostream>

#include "boost/filesystem.hpp"
#include "boost/uuid/uuid.hpp"
#include "boost/uuid/uuid_generators.hpp"
#include "boost/uuid/uuid_io.hpp"
#include "boost/lexical_cast.hpp"

namespace Tools {
    bool isRegularFile(const std::string &str) {
        return boost::filesystem::is_regular_file(boost::filesystem::path(str));
    }

    bool isDirectory(const std::string &folderPath) {
        return boost::filesystem::is_directory(
            boost::filesystem::path(folderPath));
    }

    bool createDirectory(const std::string &folderPath) {
        const boost::filesystem::path folder(folderPath);
        return (boost::filesystem::is_directory(folder))
                   ? (true)
                   : (boost::filesystem::create_directories(folder));
    }

    bool remove(const std::string &folderName) {
        const boost::filesystem::path p(folderName);
        if (boost::filesystem::exists(p)) {
            if (boost::filesystem::is_directory(p)) {
                return boost::filesystem::remove_all(p);
            } else {
                return boost::filesystem::remove(p);
            }
        }
        return false;
    }

    const std::string getCurrentFolder() {
        return boost::filesystem::current_path().string();
    }

    std::string getAbslutePath(const std::string &pathName) {
        const boost::filesystem::path path(pathName);
        return boost::filesystem::canonical(path).string();
    }

    const std::string getUniqueString() {
        return boost::lexical_cast<std::string>(
            boost::uuids::random_generator()());
    }

    /**
     * This function will copy the content of the srcFolder to desFolder
     * recursively. Note that this function may throw.
     *
     * @param srcFolder
     * @param desFolder
     */
    void copyDir(const boost::filesystem::path &srcFolder,
                 const boost::filesystem::path &desFolder) {
        using namespace boost::filesystem;
        copy_directory(srcFolder, desFolder);
        recursive_directory_iterator endIter;
        recursive_directory_iterator dirIter(srcFolder);
        for (; dirIter != endIter; ++dirIter) {
            auto aFile = dirIter->path();
            if (is_directory(aFile)) {
                auto aPath = desFolder / aFile.filename();
                copyDir(aFile, aPath);
            } else {
                auto desFile = desFolder / aFile.filename();
                copy(aFile, desFile);
            }
        }
    }

    std::string findParent(const std::vector<std::string> &allKeys,
                           const std::string &aPath) {
        auto currentItem = std::find(allKeys.begin(), allKeys.end(), aPath);
        if (currentItem == allKeys.end()) {
            auto aFolder = boost::filesystem::canonical(aPath).parent_path();
            while (!aFolder.empty()) {
                auto currentItem =
                    std::find(allKeys.begin(), allKeys.end(), aFolder.string());
                if (currentItem != allKeys.end()) {
                    break;
                } else {
                    aFolder =
                        boost::filesystem::canonical(aFolder).parent_path();
                }
            }
            return aFolder.string();
        } else {
            return *currentItem;
        }
    }

    boost::filesystem::path
    getSandboxRoot(const boost::filesystem::path &aPath) {
        const boost::filesystem::path sbtool(".sbtools");
        auto sandbox = boost::filesystem::canonical(aPath);
        while (!sandbox.empty() &&
               !boost::filesystem::exists(sandbox / sbtool)) {
            sandbox = sandbox.parent_path();
        }
        return sandbox;
    }

    class TemporaryDirectory {
      public:
        TemporaryDirectory() {
            CurrentDir = boost::filesystem::temp_directory_path() /
                         boost::filesystem::unique_path("%%%%-%%%%-%%%%-%%%%");
            boost::filesystem::create_directories(CurrentDir);
        }

        TemporaryDirectory(const std::string &parentDir) {
            CurrentDir = boost::filesystem::path(parentDir) /
                         boost::filesystem::path(getUniqueString());
            boost::filesystem::create_directories(CurrentDir);
        }

        ~TemporaryDirectory() {
            if (boost::filesystem::exists(CurrentDir)) {
                boost::filesystem::remove_all(CurrentDir);
            }
        }

        const boost::filesystem::path &getPath() { return CurrentDir; }

      private:
        boost::filesystem::path CurrentDir;
    };

    std::vector<boost::filesystem::path>
    getFilesFromTxtFile(const boost::filesystem::path &dataFile,
                        bool verbose = false) {
        std::vector<boost::filesystem::path> results;
        std::ifstream input(dataFile.string());
        for (std::string aLine; getline(input, aLine);) {
            const auto aFile = boost::filesystem::path(aLine);
            boost::system::error_code errcode;
            if (boost::filesystem::is_regular_file(aFile, errcode)) {
                results.emplace_back(aFile);
            } else {
                if (verbose)
                    std::cout << aFile << ": " << errcode.message() << "\n";
            }
        }
        return results;
    }
}
#endif
