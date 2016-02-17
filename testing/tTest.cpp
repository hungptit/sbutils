#include <array>
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <set>
#include <map>
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
#include "utils/FolderDiff.hpp"

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
        utils::Timer timer;
        utils::FileSearchBase<utils::DFSFileSearchBase> finder;

        std::cout << "Number of file: " << finder.search(aPath) << std::endl;

        for (auto aFile : finder.filter(stems, exts)) {
            std::cout << aFile << "\n";
        }

        std::cout << "Total time: " << timer.toc() / timer.ticksPerSecond()
                  << " seconds" << std::endl;
    }

    {
        utils::Timer timer;
        typedef utils::FileSearchBase<utils::BFSFileSearchBase> FileSearch;
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
        utils::Timer timer;
        typedef utils::FileSearchBase<utils::DFSFileSearchBase> Boo;
        typedef utils::BasicFileSearch<Boo, std::vector<std::string>, std::vector<std::string>> Foo;
        typedef utils::BasicFileSearch<utils::DFSFileSearchBase, std::vector<std::string>, std::vector<std::string>> Hoo;

        // Foo finder(stems, exts);
        Foo finder(stems, exts);
        std::cout << "Number of file: " << finder.search(aPath) << std::endl;
        std::cout << "Total time: " << timer.toc() / timer.ticksPerSecond()
                  << " seconds" << std::endl;
    }

    {                    
      typedef std::vector<utils::FileInfo> Container;
      const std::string dataFile = ".database";
      utils::Reader reader(dataFile);
      utils::FolderDiff<Container> diff;
      const std::string aKey("/local-ssd/sandbox/exportfcns");

      typedef utils::FileSearchBase<utils::BFSFileSearchBase> FileSearch;
      FileSearch finder;

      // Find files
      utils::Timer timer;
      diff.find(finder, aKey);
      std::cout << "Find time: " << timer.toc() / timer.ticksPerSecond()
                << " seconds" << std::endl;
      // utils::print(finder.getData());
      
      // Read data
      timer.tic();
      auto dict = diff.read(reader, aKey);
      std::cout << "Read time: " << timer.toc() / timer.ticksPerSecond()
                << " seconds" << std::endl;
      // utils::print(dict);
      
      // Find the diff
      timer.tic();
      Container editedFiles, newFiles, deletedFiles;
      std::tie(editedFiles, newFiles, deletedFiles) = diff.diff(finder.getData(), dict);   
      std::cout << "Diff time: " << timer.toc() / timer.ticksPerSecond()
                << " seconds" << std::endl;   

      std::cout << "---- Modified files: "
                << editedFiles.size() << " ----\n";
      utils::print(editedFiles);
      
      std::cout << "---- New files: "
                << newFiles.size() << " ----\n";
      utils::print(newFiles);

      std::cout << "---- Deleted files: "
                << deletedFiles.size() << " ----\n";
      utils::print(deletedFiles);
    }
}
