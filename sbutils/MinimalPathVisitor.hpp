#pragma once

#include "boost/filesystem.hpp"
#include <string>
#include <unordered_set>
#include <vector>

namespace sbutils {
    // A minimal path visitor which only handles path.
    template <typename String, typename PathContainer> class MinimalFileVisitor {
      public:
        using path = boost::filesystem::path;
        using path_container = PathContainer;
        using file_type = boost::filesystem::file_type;
		using container_type = std::vector<String>;
		using dictionary_type = typename std::unordered_set<String>;
		
        // Results
        container_type Paths;

        // Explore the first level of a given folder.
        void visit(const path &aPath, PathContainer &stack) {
            using directory_iterator = boost::filesystem::directory_iterator;
            directory_iterator endIter;
            boost::system::error_code errcode, no_error;

            // Return early if we cannot construct the directory iterator.
            directory_iterator dirIter(aPath, errcode);

            if (errcode != no_error) {
                if (errcode == boost::system::errc::not_a_directory) {
					processFile(aPath);
                }
                return;
            }

            for (; dirIter != endIter; ++dirIter) {
                auto const &currentPath = dirIter->path();
                auto const currentPathStr = currentPath.string();
                auto const status = dirIter->status(errcode);
                if (errcode != no_error) {
                    continue; // Move on if we cannot get the status of a given path.
                }

                const file_type ftype = status.type();

                if (ftype == boost::filesystem::regular_file) {
					processFile(currentPath);
                    continue;
                }

                if (ftype == boost::filesystem::directory_file) {
					processDirectory(std::move(currentPath), stack);
                }
            }
        }

      protected:
		dictionary_type Status;

		template <typename T> void updateSearchResults(T &&pathStr) {
            Paths.emplace_back(std::move(pathStr));
        }

		template<typename T>
		void processFile(T &&aPath) {
			updateSearchResults(std::move(aPath.string()));
		}

		template <typename T>
		void processDirectory(T &&aPath, path_container &stack) {
			auto const pathStr = aPath.string();
			auto const it = Status.find(pathStr);
			// Only dive into the current folder if it has not been visited yet.
			if (it == Status.end()) {
				Status.emplace_hint(it, pathStr);
				updateSearchResults(std::move(pathStr));
				stack.emplace_back(aPath);
			}

		}
    };
} // namespace sbutils
