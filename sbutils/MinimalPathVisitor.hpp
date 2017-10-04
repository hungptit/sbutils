#pragma once

#include "boost/filesystem.hpp"
#include <string>
#include <unordered_set>
#include <vector>

namespace sbutils {
    // A minimal path visitor which only handles path.
    template <typename Base> class MinimalPathVisitor : public Base {
      public:
        using string_type = typename Base::string_type;
        using path = typename Base::path;
        using path_container = typename Base::path_container;
        using container_type = std::vector<string_type>;
        using dictionary_type = typename std::unordered_set<string_type>;

        // Results
        container_type Paths;

      protected:
        dictionary_type Status;

        void updateSearchResults(const string_type &pathStr) {
            Paths.emplace_back(std::move(pathStr));
        }

        void processFile(const path &aPath) {
            updateSearchResults(std::move(aPath.string()));
        }

        void processDirectory(const path &aPath, path_container &stack) {
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
