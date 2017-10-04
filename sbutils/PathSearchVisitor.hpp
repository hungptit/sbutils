#pragma once

#include "boost/filesystem.hpp"
#include <string>
#include <vector>

namespace sbutils {
    // Simple visitor which only handle path, stem, and extension.
    template <typename Base, typename DirectoryFilterT, typename FileFilterT>
    class PathSearchVisitor : public Base {
      public:
        using string_type = typename Base::string_type;
        using path = typename Base::path;
        using path_container = typename Base::path_container;

        using container_type = std::vector<string_type>;
        using dictionary_type = std::unordered_set<string_type>;

        std::vector<string_type> Paths; // Found paths

        template <typename T1, typename T2>
        PathSearchVisitor(T1 &&dirFilter, T2 &&fileFilter, string_type &&pattern)
            : DirectoryFilter(std::forward<T1>(dirFilter)),
              FileFilter(std::forward<T2>(fileFilter)), Pattern(pattern) {}

      protected:
        DirectoryFilterT DirectoryFilter;
        FileFilterT FileFilter;
        string_type Pattern;
        std::unordered_set<string_type> Status;

        void updateSearchResults(const string_type &pathStr) {
            Paths.emplace_back(std::move(pathStr));
        }

        void processFile(const path &aPath) {
			const auto pathStr = aPath.string();
			const auto aStem = aPath.stem().string();
			const auto anExtension = aPath.extension().string();
            bool isok = FileFilter.isValidStem(aStem) && FileFilter.isValidExt(anExtension) &&
                        (pathStr.find(Pattern) != string_type::npos);
            if (isok) {
				updateSearchResults(pathStr);
            }
        }

        void processDirectory(const path &aPath, path_container &stack) {
            auto const pathStr = aPath.string();
            auto const it = Status.find(pathStr);
            // Only dive into the current folder if it has not been visited yet.
            if (it == Status.end()) {
                const auto aStem = aPath.stem().string();
                const auto anExtension = aPath.extension().string();
                Status.emplace_hint(it, pathStr);
                if (DirectoryFilter.isValidStem(aStem) &&
                    DirectoryFilter.isValidExt(anExtension)) {
                    stack.emplace_back(aPath);
                }
            }
        }
    };

} // namespace sbutils
