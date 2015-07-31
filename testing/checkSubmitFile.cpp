#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#define BOOST_THREAD_VERSION 4
#include "boost/config.hpp"
#include "boost/thread.hpp"
#include "boost/thread/future.hpp"

#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"

#include "utils/Utils.hpp"
#include "utils/FindUtils.hpp"
#include "utils/LevelDBIO.hpp"
#include "Finder.hpp"

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
        std::cerr << "Need to provide the submit file\n";
    }

    std::string dataFile;
    if (vm.count("database")) {
        dataFile = vm["database"].as<std::string>();
    } else {
        dataFile = boost::filesystem::path(Tools::FileDatabaseInfo::Database).string();
    }

    std::vector<std::string> folders, stems, extensions, searchStrings;
    boost::system::error_code errcode;
    folders.emplace_back(boost::filesystem::current_path(errcode).string());

    std::vector<std::string> fileList;
    for (auto val : Tools::getFilesFromTxtFile(vm["submit-file"].as<std::string>())) {
        fileList.emplace_back(boost::filesystem::canonical(val).string());
    }

    if (verbose) {
        std::cout << "Files listed in the\"" << submitFile << "\" file:\n";
        for (auto item : fileList) {
            std::cout << item << "\n";
        }
    }

    // Find all valid edited files.
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
    searchAlg.filter();

    std::vector<std::string> editedFiles;
    ;
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

    return EXIT_SUCCESS;
}
