#include <fstream>
#include <iostream>
#include <string>
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
        ("submit-file,s", po::value<std::string>(), "A text file which has a list of files need to remove.")
        ("folders,f", po::value<std::vector<std::string>>(), "Folders want to search.")
        ("extensions,e", po::value<std::vector<std::string>>(), "File extensions.")
        ("database,d", po::value<std::string>(), "Edited file database.");
    // clang-format on

    po::positional_options_description p;
    p.add("submit-file", -1);
    po::variables_map vm;
    po::store(
        po::command_line_parser(argc, argv).options(desc).positional(p).run(),
        vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << "Usage: deleteFiles [options]\n";
        std::cout << desc;
        std::cout << "Examples:" << std::endl;
        std::cout << "\t deleteFiles foo.txt test.txt" << std::endl;
        return 0;
    }

    bool verbose = false;
    if (vm.count("verbose")) {
        verbose = true;
    }

    std::string submitFile;
    if (vm.count("submit-file")) {
        submitFile = vm["submit-file"].as<std::string>();
    } else {
        std::cerr << "Need to provide a submit file name\n";
        return -1;
    }

    std::string dataFile;
    if (vm.count("database")) {
        dataFile = vm["database"].as<std::string>();
    } else {
        dataFile =
            boost::filesystem::path(Tools::FileDatabaseInfo::Database).string();
    }

    // Get a list of folders users want to check again files listed in the
    // submit file.
    boost::system::error_code errcode;
    std::vector<std::string> folders;
    if (vm.count("folders")) {
        for (auto item : vm["folders"].as<std::vector<std::string>>()) {
            folders.emplace_back(item);
        }
    } else {
        // folders.emplace_back(boost::filesystem::current_path(errcode).string());
        folders.emplace_back(
            "matlab/"); // Use matlab/ as a default search path.
    }

    std::vector<std::string> stems, extensions, searchStrings;
    if (vm.count("extensions")) {
        extensions = vm["extensions"].as<std::vector<std::string>>();
    }

    using boost::filesystem::path;
    path aFile(submitFile);
    if (!boost::filesystem::is_regular_file(aFile)) {
        std::cerr << aFile << " isn't a regular file!\n";
        return -1;
    }

    std::vector<std::string> fileList;
    auto files = Tools::getFilesFromTxtFile(aFile.string());
    for (auto aFile : files) {
        fileList.push_back(aFile.string());
    }

    if (verbose) {
        std::cout << "Files listed in the\"" << submitFile << "\" file:\n";
        for (auto item : fileList) {
            std::cout << item << "\n";
        }
    }

    // Find all valid edited files.
    Timer timer;
    auto const params = std::make_tuple(verbose, dataFile, folders, stems,
                                        extensions, searchStrings);
    Finder<SearchAlg, Map, decltype(params)> searchAlg(params);

    boost::future<void> readThread =
        boost::async(std::bind(&decltype(searchAlg)::read, &searchAlg));
    boost::future<void> findThread =
        boost::async(std::bind(&decltype(searchAlg)::find, &searchAlg));

    readThread.wait();
    findThread.wait();
    readThread.get();
    findThread.get();

    // Filter unrelated artifacts
    searchAlg.filter();

    // Get a list of edited files.
    std::vector<std::string> editedFiles;
    for (auto item : searchAlg.getEditedFiles()) {
        editedFiles.emplace_back(std::get<0>(item));
    }

    // Find edited files which are not belong to the submit file list.
    {
        std::cout << "Edited files which are not listed in the \"" << submitFile
                  << "\" file:\n";
        std::set<std::string> lookupTable;
        for (auto &item : fileList) {
            lookupTable.insert(item);
        }

        for (auto aFile : editedFiles) {
            if (lookupTable.find(aFile) == lookupTable.end()) {
                std::cout << aFile << "\n";
            }
        }
    }

    std::cout << "Elapsed time: " << timer.toc() / timer.ticksPerSecond()
              << " seconds" << std::endl;

    return EXIT_SUCCESS;
}
