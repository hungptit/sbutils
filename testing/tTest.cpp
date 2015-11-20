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
#include "utils/DFSFileSearch.hpp"
#include "utils/BFSFileSearch.hpp"
#include "utils/FileSearch.hpp"

#include <sstream>
#include <string>
#include <vector>

int main(int argc, char *argv[]) {
    boost::filesystem::path aPath(argv[1]);
    

    {
        Timer timer;
        Utils::SimpleFileSearch<Utils::DFSFileSearchBase> finder;
        std::cout << "Number of file: " << finder.search(aPath) << std::endl;
        std::cout << "Total time: " << timer.toc() / timer.ticksPerSecond()
                  << " seconds" << std::endl;
        // for (auto aFile : finder.getData()) {
        //     std::cout << aFile << "\n";
        // }
    }

    {
        Timer timer;
        Utils::SimpleFileSearch<Utils::BFSFileSearchBase> finder;
        std::cout << "Number of file: " << finder.search(aPath) << std::endl;
        std::cout << "Total time: " << timer.toc() / timer.ticksPerSecond()
                  << " seconds" << std::endl;
        // for (auto aFile : finder.getData()) {
        //     std::cout << aFile << "\n";
        // }
    }
}
