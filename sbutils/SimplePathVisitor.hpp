#pragma once

#include "boost/filesystem.hpp"
#include <string>
#include <vector>

namespace sbutils {
    // Simple visitor which only handle path, stem, and extension.
    template <typename Base, typename DirFilter> class SimplePathVisitor : public Base {
      public:
        using string_type = typename Base::string_type;
        using path = typename Base::path;
        using path_container = typename Base::path_container;

        using container_type = std::vector<string_type>;
        using dictionary_type = std::unordered_set<string_type>;

        container_type Paths;

      protected:
        void updateSearchResults(const string_type &pathStr) {
            Paths.emplace_back(std::move(pathStr));
        }

        void processFile(const path &aPath) { updateSearchResults(std::move(aPath.string())); }

        void processDirectory(const path &aPath, path_container &stack) {
            auto const pathStr = aPath.string();
            auto const it = Status.find(pathStr);
            // Only dive into the current folder if it has not been visited yet.
            if (it == Status.end()) {
                const auto aStem = aPath.stem().string();
                const auto anExtension = aPath.extension().string();
                Status.emplace_hint(it, pathStr);
                if (FolderFilter.isValidStem(aStem) && FolderFilter.isValidExt(anExtension)) {
                    stack.emplace_back(aPath);
                }
            }
        }

      protected:
        dictionary_type Status;
        DirFilter FolderFilter;
    };

} // namespace sbutils
