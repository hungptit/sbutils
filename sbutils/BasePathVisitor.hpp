#pragma once

#include "boost/filesystem.hpp"
#include <string>
#include <vector>
#include <unordered_set>

namespace sbutils {
    // This is the virtual base class for the path visitor.
    template <typename String, typename PathContainer> class BasePathVisitor {
      public:
        using string_type = String;
        using path = boost::filesystem::path;
        using path_container = PathContainer;
        using file_type = boost::filesystem::file_type;
        using directory_iterator = boost::filesystem::directory_iterator;
        using dictionary_type = std::unordered_set<String>;

        bool isVisited(const std::string &aPath) {
            return Visited.find(aPath) != Visited.end();
        }

        // Explore the first level of a given folder.
        void visit(const path &aPath, PathContainer &stack) {
            directory_iterator endIter;
            boost::system::error_code errcode, no_error;

            // Skip this path if we have already visited.
			auto it = Visited.find(aPath.string());
			if (it == Visited.end()) {
				Visited.emplace_hint(it, aPath.string());
			} else {
				return;
			}

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
                    processDirectory(currentPath, stack);
                }
            }
        }

      protected:
        dictionary_type Visited;
        virtual void updateSearchResults(const string_type &pathStr) = 0;
        virtual void processFile(const path &aPath) = 0;
        virtual void processDirectory(const path &aPath, path_container &stack) = 0;
    };
} // namespace sbutils
