#include <array>
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#define BOOST_THREAD_VERSION 4
#include "boost/config.hpp"
#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"
#include "boost/thread.hpp"
#include "boost/thread.hpp"
#include "boost/thread/future.hpp"

#include "utils/FindUtils.hpp"
#include "utils/LevelDBIO.hpp"
#include "utils/Timer.hpp"
#include "utils/Utils.hpp"

#include <sstream>
#include <string>
#include <vector>

namespace Tools {
    class DFSFinder {
      public:
        typedef boost::filesystem::path path;
        typedef boost::filesystem::directory_iterator directory_iterator;

        size_t search(const path &aPath) {
            size_t counter = 0;
            std::stack<path, std::vector<path>> folders;
            folders.push(aPath);
            while (!folders.empty()) {
                auto aPath = folders.top();
                folders.pop();
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
                            folders.push(currentPath);
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

    template <typename Base> class SimpleDFSFinder : public Base {
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
        void unexpected(const path &aPath) {
            std::cout << "How to handle this: " << aPath << std::endl;
        };

      private:
        std::vector<Tools::EditedFileInfo> Data;
    };
}

int main(int argc, char *argv[]) {
    boost::filesystem::path aPath(argv[1]);

    {
        Timer timer;
        Tools::SimpleDFSFinder<Tools::DFSFinder> finder;
        std::cout << "Number of file: " << finder.search(aPath) << std::endl;
        std::cout << "Total time: " << timer.toc() / timer.ticksPerSecond()
                  << " seconds" << std::endl;
        // for (auto aFile : finder.getData()) {
        //     std::cout << aFile << "\n";
        // }
    }
}
