#ifndef FileSearch_hpp_
#define FileSearch_hpp_

// STL headers
#include <array>
#include <string>
#include <vector>

// Boost libraries
#define BOOST_THREAD_VERSION 4
#include "boost/config.hpp"
#include "boost/filesystem.hpp"

#include "cppformat/format.h"
#include "Utils.hpp"
#include "FileUtils.hpp"

namespace utils {
    /**
     * Find all file using the algorithm provided by the base class.
     *
     */
using FileInfo = std::tuple<std::string, std::string, std::string, int, std::time_t>;
  
  template <typename Base> class FileSearchBase : public Base {
      public:
        explicit FileSearchBase() {}

        explicit FileSearchBase(size_t size) { Data.reserve(size); }

        auto getData() { return Data; }

        template <typename StemContainer, typename ExtContainer>
        std::vector<FileInfo> filter(StemContainer &stems,
                                            ExtContainer &exts) {
          std::vector<FileInfo> results;
            for (auto item : Data) {
                auto stem = std::get<1>(item);
                auto ext = std::get<2>(item);
                bool flag = (std::find(stems.begin(), stems.end(),
                                       std::get<1>(item)) == stems.end()) &&
                            ((std::find(exts.begin(), exts.end(),
                                        std::get<2>(item)) == exts.end()));
                if (flag) {
                    results.emplace_back(item);
                }
            }
            return results;
        }

      protected:
        typedef typename Base::path path;
        bool isValidDir(const path &aPath) {
            const std::array<std::string, 1> excludedFolders = {{".git"}};
            auto searchStr = aPath.extension().string();
            return std::find(excludedFolders.begin(), excludedFolders.end(),
                             searchStr) == excludedFolders.end();
        };

        bool isValidFile(const path &) { return true; };

        void update(const path &aPath, boost::filesystem::file_status &fs) {
            Data.emplace_back(
                std::make_tuple(aPath.string(), aPath.stem().string(),
                                aPath.extension().string(), fs.permissions(),
                                boost::filesystem::last_write_time(aPath)));
        };

        void unexpected(const path &){
            // TODO: How could we handle symlink files?
        };

      private:
    std::vector<FileInfo> Data;
    };

    template <typename Base, typename StemContainer, typename ExtContainer>
    class BasicFileSearch : public Base {
      public:
        BasicFileSearch(StemContainer &stems, ExtContainer &exts)
            : Stems(stems), Exts(exts) {}

        auto getData() { return Data; }

      protected:
        typedef typename Base::path path;

        bool isValidDir(const path &aPath) {
            const std::array<std::string, 1> excludedFolders = {{".git"}};
            auto searchStr = aPath.extension().string();
            return std::find(excludedFolders.begin(), excludedFolders.end(),
                             searchStr) == excludedFolders.end();
        };

        bool isValidFile(const path &aPath) {
            return (Stems.empty() ||
                    (std::find(Stems.begin(), Stems.end(),
                               aPath.stem().string()) != Stems.end())) &&
                   (Exts.empty() ||
                    (std::find(Exts.begin(), Exts.end(),
                               aPath.extension().string()) != Exts.end()));
        };

        void update(const path &aPath, boost::filesystem::file_status &fs) {
            Data.emplace_back(
                std::make_tuple(aPath.string(), aPath.stem().string(),
                                aPath.extension().string(), fs.permissions(),
                                boost::filesystem::last_write_time(aPath)));
        };

        void unexpected(const path &){
            // TODO: How could we handle symlink files?
        };

      private:
      std::vector<FileInfo> Data;
        StemContainer Stems;
        ExtContainer Exts;
    };
}

#endif
