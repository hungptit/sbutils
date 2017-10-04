#pragma once

#include "boost/filesystem.hpp"
#include <string>
#include <vector>

namespace sbutils {
    // Simple visitor which only handle path, stem, and extension.
    template <typename String, typename PathContainer, typename DirectoryFilter>
    class SimpleFileVisitor {
      public:
        using path = boost::filesystem::path;
        using directory_iterator = boost::filesystem::directory_iterator;
        using path_container = PathContainer;
        using file_type = boost::filesystem::file_type;

        std::vector<String> Paths; // Found paths

        // Explore the first level of a given path.
        void visit(const path &aPath, PathContainer &stack) {
            namespace fs = boost::filesystem;
            directory_iterator endIter;
            boost::system::error_code errcode, no_error;

            // Return early if we cannot construct the directory iterator.
            directory_iterator dirIter(aPath, errcode);
			if (errcode == boost::system::errc::file_exists) {
				std::cout << "visit: " << errcode << "\n";
			}
			
            if (errcode != no_error) {
                return;
            }

			std::cout << "visit: " << aPath << "\n";
			
            for (; dirIter != endIter; ++dirIter) {
                auto const currentPath = dirIter->path();
                auto const currentPathStr = currentPath.string();

                auto const status = dirIter->status(errcode);
                if (errcode != no_error) {
                    continue; // Move on if we cannot get the status of a given path.
                }

                const file_type ftype = status.type();
                const auto aStem = currentPath.stem().string();
                const auto anExtension = currentPath.extension().string();

                if (ftype == boost::filesystem::regular_file) {
                    Paths.emplace_back(currentPathStr);
                    continue;
                }

                if (ftype == boost::filesystem::directory_file) {
                    auto const it = Status.find(currentPathStr);
					// Only dive into the current folder if it has not been visited yet.
                    if (it == Status.end()) {
                        Status.emplace_hint(it, currentPathStr);
                        if (FolderFilter.isValidStem(aStem) &&
                            FolderFilter.isValidExt(anExtension)) {
                            Paths.emplace_back(currentPath.string());
                            stack.emplace_back(currentPath);
                        }
                    }
                }
            }
        }

      private:
        DirectoryFilter FolderFilter;
        std::unordered_set<String> Status;
    };

} // namespace sbutils
