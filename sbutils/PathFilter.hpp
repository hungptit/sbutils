#pragma once

// This file has policy classes for filtering paths.

#include <array>
#include <string>

namespace sbutils {
    struct NullPolicy {
        template <typename String> bool isValidStem(String &&) { return true; }
        template <typename String> bool isValidExt(String &&) { return true; }
    };

    // A file search policy.
    template <typename String, typename Container> struct NormalPolicy {
        NormalPolicy(Container &&stems, Container &&exts)
            : Stems(std::forward<Container>(stems)), Extensions(std::forward<Container>(exts)) {}

        template <typename T> bool isValidExt(T &&anExtension) {
            if (Extensions.empty()) return true;
            return std::find(Extensions.begin(), Extensions.end(), anExtension) !=
                   Extensions.end();
        }

        template <typename T> bool isValidStem(T &&aStem) {
            if (Stems.empty()) return true;
            return std::find(Stems.begin(), Stems.end(), aStem) != Stems.end();
        }

        Container Stems;
        Container Extensions;
    };

	// A policy class for searching Perl files.
    class PerlPolicy {
      public:
        template <typename T> bool isValidExt(T &&anExtension) {
            return std::find(Extensions.begin(), Extensions.end(), anExtension) !=
                   Extensions.end();
        }

        template <typename T> bool isValidStem(T &&) { return true; }

      private:
        const std::array<std::string, 3> Extensions = {{".pm", ".pl", ".t"}};
    };

    class CppPolicy {
      public:
        template <typename T> bool isValidExt(T &&anExtension) {
            return std::find(Extensions.begin(), Extensions.end(), anExtension) !=
                   Extensions.end();
        }

        template <typename T> bool isValidStem(T &&) { return true; }

      private:
        const std::array<std::string, 5> Extensions = {{".c", ".cpp", ".cc", ".hpp", ".h"}};
    };

    class FolderPolicy {
      public:
        template <typename T> bool isValidExt(T &&anExtension) {
            return std::find(ExcludedExtensions.begin(), ExcludedExtensions.end(),
                             anExtension) == ExcludedExtensions.end();
        }

        template <typename T> bool isValidStem(T &&aStem) {
            return std::find(ExcludedStems.begin(), ExcludedStems.end(), aStem) ==
                   ExcludedStems.end();
        }

      private:
        const std::array<std::string, 2> ExcludedExtensions = {{".git", ".cache"}};
        const std::array<std::string, 2> ExcludedStems = {{"CMakeFiles", "CMakeTmp"}};
    };
} // namespace sbutils
