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
#include "utils/Utils.hpp"
#include "utils/Timer.hpp"

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
                    if ((boost::filesystem::is_directory(currentPath))) {
                        if (isValidDir(currentPath)) {
                            folders.push(currentPath);
                        }
                    } else if ((boost::filesystem::is_regular_file(
                                   currentPath))) {
                        if (isValidFile(currentPath)) {
                            update(currentPath);
                            counter ++;
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
        virtual void update(const path &aPath) = 0;
        virtual void unexpected(const path &aPath) = 0;
    };

    template <typename Base> class SimpleDFSFinder : public Base {
      public:
      protected:
        typedef typename Base::path path;
        bool isValidDir(const path &aPath) {return true;};
        bool isValidFile(const path &aPath) {return true;};
        void update(const path &aPath) {};
        void unexpected(const path &aPath) {};
      private:
        
    };
}

int main() {
    Timer timer;
    Tools::SimpleDFSFinder<Tools::DFSFinder> finder;
    std::cout << "Number of file: " << finder.search("/local-ssd/sandbox/exportfcns") << std::endl;;
    std::cout << "Total time: " << timer.toc() / timer.ticksPerSecond()
              << " seconds" << std::endl;
}
