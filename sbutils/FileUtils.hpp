#pragma once

// Boost libraries
#include "boost/date_time/gregorian/gregorian.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/filesystem.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/uuid/uuid.hpp"
#include "boost/uuid/uuid_generators.hpp"
#include "boost/uuid/uuid_io.hpp"
#include <boost/iostreams/device/mapped_file.hpp> // for readLines

namespace sbutils {
    // Text file I/O
    std::vector<std::string> readLines(const std::string &dataFile) {
        boost::iostreams::mapped_file mmap(
            dataFile, boost::iostreams::mapped_file::readonly);
        auto begin = mmap.const_data();
        auto end = begin + mmap.size();

        std::vector<std::string> results;
        while (begin && begin != end) {
            auto currentPos = begin;
            while ((*currentPos != '\n') && (currentPos != end)) {
                ++currentPos;
            }
            results.emplace_back(std::string(begin, currentPos));
            begin = ++currentPos;
        }
        return results;
    }

    std::vector<std::string> readLines(const std::string &dataFile,
                                       size_t startLine, size_t stopLine) {
        boost::iostreams::mapped_file mmap(
            dataFile, boost::iostreams::mapped_file::readonly);
        auto begin = mmap.const_data();
        auto end = begin + mmap.size();

        std::vector<std::string> results;
        size_t linenum = 0;
        while (begin && begin != end) {
            auto currentPos = begin;
            while ((*currentPos != '\n') && (currentPos != end)) {
                ++currentPos;
            }
            ++linenum;
            if ((linenum >= startLine) && (linenum <= stopLine)) {
                results.emplace_back(std::string(begin, currentPos));
            }
            begin = ++currentPos;
        }
        return results;
    }

	template<typename Container>
    std::string read(const std::string &textFile) {
        boost::iostreams::mapped_file mmap(
            textFile, boost::iostreams::mapped_file::readonly);
        auto begin = mmap.const_data();
        auto end = begin + mmap.size();
        return Container(begin, end);
    }

    std::vector<boost::filesystem::path>
    getFilesFromTxtFile(const boost::filesystem::path &dataFile,
                        bool verbose = false) {
        std::vector<boost::filesystem::path> results;
        for (auto aLine : readLines(dataFile.string())) {
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

    /** 
     * Remove trailing slash characters
     *
     * @param aPath 
     *
     * @return 
     */
    std::string normalize_path(const std::string &aPath) {
        using namespace boost::filesystem;
        auto pos = aPath.size() - 1;
        auto sepChar = path::preferred_separator;
        for (; pos != 0; --pos) {
            if (aPath[pos] != sepChar)
                break;
        }
        return aPath.substr(0, pos + 1);
    }

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
                auto subItem =
                    std::find(allKeys.begin(), allKeys.end(), aFolder.string());
                if (subItem != allKeys.end()) {
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

    boost::filesystem::path getPath(const boost::filesystem::path &aPath,
                                    bool useRelativePath) {
        if (useRelativePath) {
            auto tmpPath = aPath.stem();
            tmpPath += aPath.extension();
            return tmpPath;
        } else {
            return boost::filesystem::canonical(aPath);
        }
    }

    std::tuple<std::vector<boost::filesystem::path>,
               std::vector<boost::filesystem::path>>
    exploreFolderAtRootLevel(const boost::filesystem::path &aPath,
                             bool useRelativePath) {
        std::vector<boost::filesystem::path> files;
        std::vector<boost::filesystem::path> folders;

        // Explore a given folder at the root level.
        boost::filesystem::directory_iterator endIter;
        boost::filesystem::directory_iterator dirIter(aPath);
        for (; dirIter != endIter; ++dirIter) {
            auto currentPath = dirIter->path();
            if (boost::filesystem::is_directory(currentPath)) {
                folders.push_back(getPath(currentPath, useRelativePath));
            } else if (boost::filesystem::is_regular_file(currentPath)) {
                files.push_back(getPath(currentPath, useRelativePath));
            }
        }
        return std::make_tuple(folders, files);
    }

    // Time related functionalities
    std::string GetTimeStampString() {
        const boost::posix_time::ptime now =
            boost::posix_time::second_clock::local_time();
        return boost::posix_time::to_iso_string(now);
    }
}
