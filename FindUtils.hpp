#ifndef FindUtils_hpp
#define FindUtils_hpp

#include <string>
#include <tuple>
#include <vector>

#include "boost/lexical_cast.hpp"
#include "boost/regex.hpp"
#include <boost/filesystem.hpp>

#include "Utils.hpp"

namespace Utils {
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

    // Explore a folder to a given level. 
    // TODO: Need to rewrite this algorithm base on the BFS algorithms.
    std::tuple<std::vector<boost::filesystem::path>,
               std::vector<boost::filesystem::path>>
    exploreFolders(size_t level, const boost::filesystem::path &rootFolder,
                   bool useRelativePath = false) {
        auto results = exploreFolderAtRootLevel(rootFolder, useRelativePath);
        std::vector<boost::filesystem::path> files = std::get<1>(results);
        std::vector<boost::filesystem::path> folders = std::get<0>(results);
        size_t counter = 1;

        // This code does not make any assumtion about the input path.
        while (counter < level) {
            decltype(folders) nextLevel;
            for (auto const &aPath : folders) {
                boost::filesystem::directory_iterator endIter;
                boost::filesystem::directory_iterator dirIter(aPath);
                for (; dirIter != endIter; ++dirIter) {
                    auto currentPath = dirIter->path();
                    if (boost::filesystem::is_directory(currentPath)) {
                        nextLevel.push_back(currentPath);
                    } else if (boost::filesystem::is_regular_file(
                                   currentPath)) {
                        files.push_back(currentPath);
                    }
                }
            }

            if (nextLevel.empty()) {
                break;
            } else {
                folders.reserve(nextLevel.size());
                // Move content of nextLevel to folders then clear nextLevel
                // content.
                folders = std::move(nextLevel);
                counter++;
            }
        }
        return std::make_tuple(folders, files);
    }
}
#endif
