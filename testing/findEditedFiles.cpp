#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <tuple>
#include <unordered_map>

#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"

#include "utils/Utils.hpp"
#include "utils/FindUtils.hpp"
#include "utils/LevelDBIO.hpp"

#include "InputArgumentParser.hpp"

#include <string>
#include <vector>
#include <map>
#include <ctime>
#include <sstream>

typedef Tools::DefaultOArchive OArchive;
typedef Tools::DefaultIArchive IArchive;
typedef Tools::EditedFileInfo FileInfo;
typedef std::unordered_map<std::string, Tools::EditedFileInfo> Map;

template <typename SearchAlg> auto getEditedFiles(const Tools::InputArgumentParser &params) {
    SearchAlg fSearch(params.Extensions);
    for (const auto &aFolder : params.Folders) {
        // const std::string aPath = Tools::getAbslutePath(aFolder);
        fSearch.search(boost::filesystem::canonical(aFolder));
    }

    auto data = fSearch.getData();

    if (params.Verbose) {
        std::cout << "==== Search results ====\n";
        std::cout << "Number of files: " << data.size() << '\n';
        for (auto const &info : data) {
            std::cout << std::get<0>(info) << "\n";
        }
    }
    return data;
}


template <typename FileInfo>
std::vector<FileInfo> read(const Tools::InputArgumentParser &params, const size_t maxlen = 1200000) {
    std::vector<FileInfo> database;
    database.reserve(maxlen);
    if (!params.Database.empty()) {
        Tools::Reader reader(params.Database);
        auto allKeys = reader.keys();

        if (params.Verbose) {
            std::cout << "All keys in the " << params.Database << " database: " << std::endl;
            for (auto const &info : allKeys) {
                std::cout << info << "\n";
            }
        }       
        
        auto aKey = Tools::findParent(allKeys, params.Folders[0]);
        auto const &results = reader.read(aKey);
        if (!results.empty()) {
            std::istringstream is(results);
            Tools::load<IArchive, decltype(database)>(database, is);
        }

        if (params.Verbose) {
            std::cout << "Number of files in edited database: " << database.size() << std::endl;
        }
    }
    return database;
}

template <typename Map> auto createLookupTable(const Tools::InputArgumentParser &params) {
    typedef typename Map::mapped_type FileInfo;
    auto const database = read<FileInfo>(params);
    Map lookupTable;

    for (auto const &item : database) {
        auto aKey = std::get<0>(item);
        lookupTable.emplace(aKey, item);
    }

    if (params.Verbose) {
        std::cout << "Look up table size: " << lookupTable.size() << std::endl;
    }

    return lookupTable;
}

template <typename Map>
auto filter(const Map &lookupTable, const std::vector<typename Map::mapped_type> &data,
            const Tools::InputArgumentParser &) {
    typedef typename Map::mapped_type FileInfo;
    std::vector<FileInfo> editedFiles;
    for (const auto &anEditedFile : data) {
        auto aKey = std::get<0>(anEditedFile);
        auto aFile = lookupTable.find(aKey);
        if ((aFile == lookupTable.end()) || (anEditedFile != aFile->second)) {
            // If we could not find a given key in the database or the value
            // associated with that key is the the same with the current value
            // then we need to record this file.
            editedFiles.emplace_back(anEditedFile);
        }
    }
    return editedFiles;
}

template <typename Container> void print(const Tools::InputArgumentParser &params, Container &editedFiles) {
    if (params.Verbose) {
        std::cout << "Edited files: " << std::endl;
        for (const auto &val : editedFiles) {
            std::cout << val << std::endl;
        }
    } else {
        // TODO: Below constraints are specific to the MW sandbox so we need to
        // store them in database?
        std::array<std::string, 4> excludedStems = {{"/.sbtools/", "/derived/", "toolbox_cache-glnxa64", "~"}};
        std::array<std::string, 3> excludedExtensions = {{".p", ".so", ".dbg"}};

        for (const auto &val : editedFiles) {
            bool isExcluded = false;
            auto aPath = std::get<0>(val);
            isExcluded = std::any_of(excludedExtensions.begin(), excludedExtensions.end(),
                                     [=](const std::string &extStr) { return extStr == std::get<2>(val); }) ||
                         std::any_of(excludedStems.begin(), excludedStems.end(),
                                     [=](const std::string &extStr) { return aPath.find(extStr) != std::string::npos; });

            if (!isExcluded) {
                std::cout << std::get<0>(val) << std::endl;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    Tools::InputArgumentParser params(argc, argv);
    if (params.Verbose) {
        params.disp();
    }
    if (!params.Help) {
        auto data = getEditedFiles<Tools::FindEditedFiles<Tools::Finder>>(params); // Get the list of edited files.
        auto lookupTable = createLookupTable<Map>(params);                         // Load the lookup table from the database
        auto editedFiles = filter(lookupTable, data, params);                      // Filter edited file list
        print(params, editedFiles);
    }
    return 0;
}
