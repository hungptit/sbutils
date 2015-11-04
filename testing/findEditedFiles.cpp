#include <array>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#define BOOST_THREAD_VERSION 4
#include "boost/config.hpp"
#include "boost/thread.hpp"
#include "boost/thread/future.hpp"

#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"

#include "Finder.hpp"
#include "utils/FindUtils.hpp"
#include "utils/LevelDBIO.hpp"
#include "utils/Timer.hpp"
#include "utils/Utils.hpp"

int main(int argc, char *argv[]) {
    typedef std::unordered_map<std::string, Tools::EditedFileInfo> Map;
    typedef Tools::FindEditedFiles<Tools::Finder> SearchAlg;

    using namespace boost;
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");

    // clang-format off
    desc.add_options()              
        ("help,h", "Print this help")
        ("verbose,v", "Display searched data.")
        ("use_absolute_path,a", "Use absolute path.")
        ("folders,f", po::value<std::vector<std::string>>(), "Search folders.")
        ("file-stems,s", po::value<std::vector<std::string>>(), "File stems.")
        ("extensions,e", po::value<std::vector<std::string>>(), "File extensions.")
        ("search-strings,t", po::value<std::vector<std::string>>(), "File extensions.")
        ("regexp,r", po::value<std::vector<std::string>>(), "Search using regular expression.")
        ("database,d", po::value<std::string>(), "File database.");
    // clang-format on

    po::positional_options_description p;
    p.add("folders", -1);
    po::variables_map vm;
    po::store(
        po::command_line_parser(argc, argv).options(desc).positional(p).run(),
        vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc;
        std::cout << "Examples:" << std::endl;
        std::cout << "\t findEditedFiles ./ -d .database" << std::endl;
        return EXIT_SUCCESS;
    }

    auto verbose = false;
    if (vm.count("verbose")) {
        verbose = true;
    }

    bool useRelativePath = true;
    if (vm.count("use_absolute_path")) {
        useRelativePath = false;
    }

    // Notes: All folder paths must be full paths.
    boost::system::error_code errcode;
    std::vector<std::string> folders;
    if (vm.count("folders")) {
        for (auto item : vm["folders"].as<std::vector<std::string>>()) {
            if (useRelativePath) {
                folders.emplace_back(item);
            } else {
                folders.emplace_back(
                    boost::filesystem::canonical(item, errcode).string());
            }
        }
    }

    std::vector<std::string> stems;
    if (vm.count("file-stems")) {
        stems = vm["file-stems"].as<std::vector<std::string>>();
    }

    std::vector<std::string> extensions;
    if (vm.count("extensions")) {
        extensions = vm["extensions"].as<std::vector<std::string>>();
    }

    std::vector<std::string> searchStrings;
    if (vm.count("search-strings")) {
        searchStrings = vm["search-strings"].as<std::vector<std::string>>();
    }

    std::string dataFile;
    if (vm.count("database")) {
        dataFile = vm["database"].as<std::string>();
    } else {
        dataFile =
            boost::filesystem::path(Tools::FileDatabaseInfo::Database).string();
    }

    Timer timer;

    // Launch read and find tasks using two async threads.
    auto const params = std::make_tuple(verbose, dataFile, folders, stems,
                                        extensions, searchStrings);
    Finder<SearchAlg, Map, decltype(params)> searchAlg(params);

    boost::future<void> readThread =
        boost::async(std::bind(&decltype(searchAlg)::read, &searchAlg));
    boost::future<void> findThread =
        boost::async(std::bind(&decltype(searchAlg)::find, &searchAlg));

    readThread.wait();
    findThread.wait();

    std::cout << "Search time: " << timer.toc() / timer.ticksPerSecond()
              << " seconds" << std::endl;

    readThread.get();
    findThread.get();

    // Get the list of edited files then print out the results.
    std::cout << "Number of new or modified files: " << searchAlg.filter()
              << "\n";

    searchAlg.disp();
    std::cout << "Total time: " << timer.toc() / timer.ticksPerSecond()
              << " seconds" << std::endl;

    return 0;
}
