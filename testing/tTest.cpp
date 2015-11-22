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

#include "utils/BFSFileSearch.hpp"
#include "utils/DFSFileSearch.hpp"
#include "utils/FileSearch.hpp"
#include "utils/LevelDBIO.hpp"
#include "utils/Timer.hpp"
#include "utils/Utils.hpp"

#include <sstream>
#include <string>
#include <vector>

#include "gtest/gtest.h"


int main(int argc, char *argv[]) {
    boost::filesystem::path aPath((argc == 2) ? argv[1] : "./");
    
    std::array<std::string, 1> stems = {{"foo"}};
    std::vector<std::string> exts = {{".txt", ".cmake", ".make", ".o", ".bin",
                                      ".internal", ".includecache", ".marks"}};

    {
        Timer timer;
        Utils::FileSearchBase<Utils::DFSFileSearchBase> finder;

        std::cout << "Number of file: " << finder.search(aPath) << std::endl;

        for (auto aFile : finder.filter(stems, exts)) {
            std::cout << aFile << "\n";
        }

        std::cout << "Total time: " << timer.toc() / timer.ticksPerSecond()
                  << " seconds" << std::endl;
    }

    {
        Timer timer;
        typedef Utils::FileSearchBase<Utils::BFSFileSearchBase> FileSearch;
        FileSearch finder;
        std::cout << "Number of file: " << finder.search(aPath) << std::endl;
        std::cout << "Total time: " << timer.toc() / timer.ticksPerSecond()
                  << " seconds" << std::endl;
        // for (auto aFile : finder.getData()) {
        //     std::cout << aFile << "\n";
        // }
    }

    {
        std::vector<std::string> stems = {{"CMakeLists"}};
        std::vector<std::string> exts = {{".txt", ".cmake", ".make", ".o", ".bin",
                                          ".internal", ".includecache", ".marks"}};
        Timer timer;
        typedef Utils::FileSearchBase<Utils::DFSFileSearchBase> Boo;
        typedef Utils::BasicFileSearch<Boo, std::vector<std::string>, std::vector<std::string>> Foo;
        typedef Utils::BasicFileSearch<Utils::DFSFileSearchBase, std::vector<std::string>, std::vector<std::string>> Hoo;

        // Foo finder(stems, exts);
        Foo finder(stems, exts);
        std::cout << "Number of file: " << finder.search(aPath) << std::endl;
        std::cout << "Total time: " << timer.toc() / timer.ticksPerSecond()
                  << " seconds" << std::endl;
    }
}
