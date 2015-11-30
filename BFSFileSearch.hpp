#ifndef BFSFileSearch_hpp_
#define BFSFileSearch_hpp_

// STL headers
#include <array>
#include <deque>
#include <queue>
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
     * This class define a file search algorithm using the depth-first search.
     * Derived class will need to overwrite virtual functions.
     */
    class BFSFileSearchBase {
      public:
        typedef boost::filesystem::path path;
        typedef boost::filesystem::directory_iterator directory_iterator;

        size_t search(const path &aPath) {
            size_t counter = 0;
            std::deque<path> folders;
            folders.push_back(aPath);
            while (!folders.empty()) {
                auto aPath = folders.front();
                folders.pop_front();
                directory_iterator endIter;
                directory_iterator dirIter(aPath);
                for (; dirIter != endIter; ++dirIter) {
                    auto currentPath = dirIter->path();
                    auto status = dirIter->status();
                    auto ftype = status.type();
                    if (ftype == boost::filesystem::regular_file) {
                        if (isValidFile(currentPath)) {
                            update(currentPath, status);
                            counter++;
                        }
                    } else if (ftype == boost::filesystem::directory_file) {
                        if (isValidDir(currentPath)) {
                            folders.push_back(currentPath);
                        }
                    } else {
                        unexpected(currentPath);
                    }
                }
            }
            return counter;
        }

      protected:
        virtual bool isValidDir(const path &aPath) = 0;
        virtual bool isValidFile(const path &aPath) = 0;
        virtual void update(const path &aPath,
                            boost::filesystem::file_status &fs) = 0;
        virtual void unexpected(const path &aPath) = 0;
    };
}

#endif
