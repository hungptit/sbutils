#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <tuple>
#include <unordered_map>

#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"
#include "boost/thread.hpp"

#include "utils/Utils.hpp"
#include "utils/FindUtils.hpp"
#include "utils/LevelDBIO.hpp"

#include "InputArgumentParser.hpp"

#include <string>
#include <vector>
#include <map>
#include <ctime>
#include <sstream>

void find(const std::string &dataFile, std::vector<std::string> &folders, std::vector<std::string> &stems,
          std::vector<std::string> &extensions, std::vector<std::string> &searchStrings) {
    Tools::Reader reader(dataFile);
    auto allKeys = reader.keys(); // TODO: Only get keys (folders) which are closest ancesstor of given folders.
    std::set<Tools::EditedFileInfo> aList;
    for (auto &aKey : allKeys) {
        std::vector<Tools::EditedFileInfo> data;
        auto buffer = reader.read(aKey);
        std::istringstream is(buffer);
        Tools::load<Tools::DefaultIArchive, decltype(data)>(data, is);
        for (auto info : data) {
            bool flag = (stems.empty() || std::find(stems.begin(), stems.end(), std::get<1>(info)) != stems.end()) &&
                        (extensions.empty() ||
                         std::find(extensions.begin(), extensions.end(), std::get<2>(info)) != extensions.end()) &&
                        (searchStrings.empty() || true);
            if (flag) {
                aList.insert(info);
            }
        }
    }

    for (auto const &aFile : aList) {
        std::cout << std::get<0>(aFile) << std::endl;
    }
}

int main(int argc, char *argv[]) {
    using namespace boost;
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");

    // clang-format off
    desc.add_options()
        ("help,h", "Print this help")
        ("verbose,v", "Display searched data.")
        ("keys,k", "List all keys.")
        ("folders,f", po::value<std::vector<std::string>>(), "Search folders.")
        ("stems,s", po::value<std::vector<std::string>>(), "File stems.")
        ("extensions,e", po::value<std::vector<std::string>>(), "File extensions.")
        ("strings,t", po::value<std::vector<std::string>>(), "Search string")
        ("database,d", po::value<std::string>(), "File database.");
    // clang-format on

    po::positional_options_description p;
    p.add("folders", -1);
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << "Usage: buildEditedFileDatabase [options]\n";
        std::cout << desc;
        std::cout << "Examples:" << std::endl;
        std::cout << "\t finder ./ -d testdata." << std::endl;
        return 0;
    }

    bool verbose = false;
    if (vm.count("verbose")) {
        verbose = true;
    }

    // Search folders
    std::vector<std::string> folders;
    if (vm.count("folders")) {
        folders = vm["folders"].as<std::vector<std::string>>();
    }

    // Get file stems
    std::vector<std::string> stems;
    if (vm.count("stems")) {
        stems = vm["stems"].as<std::vector<std::string>>();
    }

    // Get file extensions
    std::vector<std::string> extensions;
    if (vm.count("extensions")) {
        extensions = vm["extensions"].as<std::vector<std::string>>();
    }

    // Get file extensions
    std::vector<std::string> searchStrings;
    if (vm.count("strings")) {
        searchStrings = vm["strings"].as<std::vector<std::string>>();
    }

    // Get file database
    std::string dataFile;
    if (vm.count("database")) {
        dataFile = vm["database"].as<std::string>();
    } else {
        dataFile = (boost::filesystem::path(Tools::FileDatabaseInfo::Database)).string();
    }

    if (verbose) {
        std::cout << "Database: " << dataFile << std::endl;
    }

    if (vm.count("keys")) {
        Tools::Reader reader(dataFile);
        for (auto &aKey : reader.keys()) {
            std::cout << aKey << std::endl;
        }
    } else {
        find(dataFile, folders, stems, extensions, searchStrings); // View files which are satified given constraints.
    }

    return 0;
}
