#ifndef FileSearch_hpp_
#define FileSearch_hpp_

// STL headers
#include <array>
#include <string>
#include <tuple>
#include <vector>

// Boost libraries
#define BOOST_THREAD_VERSION 4
#include "boost/config.hpp"
#include "boost/filesystem.hpp"

// Headers from utils package.
#include "utils/Utils.hpp"

// CPPFormat
#include "cppformat/format.h"

namespace Utils {
    /**
     * Find all file using the algorithm provided by the base class.
     *
     */
    template <typename Base> class SimpleFileSearch : public Base {
      public:
        auto getData() { return Data; }

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

        void unexpected(const path &aPath){
            // Ignore these files now.
        };

      private:
        std::vector<Utils::FileInfo> Data;
    };
}

#endif
