#pragma once

#include <string>
#include <vector>
#include "boost/filesystem.hpp"

namespace sbutils {
    // Simple visitor which only handle path, stem, and extension.
    template <typename String, typename PathContainer, typename Filter>
    struct SimpleFileVisitor {
        using path = boost::filesystem::path;
        using directory_iterator = boost::filesystem::directory_iterator;
        using path_container = PathContainer;
        using file_type = boost::filesystem::file_type;

        // Data
        std::vector<String> Paths;
        Filter CustomFilter;
        std::unordered_set<String> Status;

        // Explore the first level of a given folder.
        void visit(const path &aPath, PathContainer &stack) {
            namespace fs = boost::filesystem;
            directory_iterator endIter;
            boost::system::error_code errcode, no_error;

            // Return early if we cannot construct the directory iterator.
            directory_iterator dirIter(aPath, errcode);
            if (errcode != no_error) {
                return;
            }

            for (; dirIter != endIter; ++dirIter) {
                auto const currentPath = dirIter->path();
                auto const currentPathStr = currentPath.string();
                auto const it = Status.find(currentPathStr);
                if (it != Status.end()) {
                    continue; // The current path has been processed.
                }

                auto const status = dirIter->status(errcode);
                if (errcode != no_error) {
                    continue; // Move on if we cannot get the status of a given path.
                }

                const file_type ftype = status.type();
                const auto aStem = currentPath.stem().string();
                const auto anExtension = currentPath.extension().string();

                switch (ftype) {
                case boost::filesystem::symlink_file:
                    // TODO: Need to find out the best way to handle symlink.
                    break;
                case boost::filesystem::regular_file:
                    Paths.emplace_back(currentPathStr);
                    break;
                case boost::filesystem::directory_file:
                    Status.emplace_hint(it, currentPathStr);
                    // We only interrested in folders that satisfy CustomFilter's constraints.
                    if (CustomFilter.isValidStem(aStem) &&
                        CustomFilter.isValidExt(anExtension)) {
                        Paths.emplace_back(currentPath.string());
                        stack.emplace_back(currentPath);
                    }
                    break;
                default:
                    // Do not know how to handle this case.
                    break;
                }
            }
        }
    };

}
