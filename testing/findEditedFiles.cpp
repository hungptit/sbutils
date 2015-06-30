#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <tuple>

#include "boost/filesystem.hpp"
#include "boost/unordered_map.hpp"
#include "boost/unordered_set.hpp"
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

std::vector<FileInfo> getEditedFiles(const Tools::InputArgumentParser &params) {
    Tools::FindEditedFiles<Tools::Finder> fSearch(params.Extensions);
    for (const auto &aFolder : params.Folders) {
        const std::string aPath = Tools::getAbslutePath(aFolder);
        fSearch.search(aPath);
    }

    auto data = fSearch.getData();

    if (params.Verbose) {
        std::cout << "Edited files: " << data.size() << std::endl;
        for (auto const &info : data) {
            std::cout << std::get<0>(info) << "\n";
        }
    }
    return data;
}

boost::unordered_map<std::string, FileInfo> loadDatabase(const Tools::InputArgumentParser &params) {
    std::vector<FileInfo> database;
    database.reserve(1200000); // Optimized for the MW sandbox
    if (!params.Database.empty()) {
        Tools::Reader reader(params.Database);

        if (params.Verbose) {
            const auto allKeys = reader.keys();
            std::cout << "All keys in the " << params.Database << " database: " << std::endl;
            for (auto const &info : allKeys) {
                std::cout << info << "\n";
            }
        }

        // Read in the desired keys
        auto const &results = reader.read(Tools::getAbslutePath(params.Folders[0]));
        if (!results.empty()) {
            std::istringstream is(results);
            Tools::load<IArchive, decltype(database)>(database, is);
        }

        if (params.Verbose) {
            std::cout << "Number of files in edited database: " << database.size() << std::endl;
        }
    }

    // Now get the list of edited files.
    boost::unordered_map<std::string, FileInfo> lookupTable;
    for (auto const &item : database) {
        auto aKey = std::get<0>(item);
        lookupTable.emplace(aKey, item);
    }

    if (params.Verbose) {
        std::cout << "Look up table size: " << lookupTable.size() << std::endl;
    }

    return lookupTable;
}

std::vector<FileInfo> filter(boost::unordered_map<std::string, FileInfo> &lookupTable, std::vector<FileInfo> &data) {
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

void print(const Tools::InputArgumentParser &params, std::vector<FileInfo> &editedFiles) {
    std::cout << "Edited files: " << std::endl;
    if (params.Verbose) {
        for (const auto &val : editedFiles) {
            std::cout << val << std::endl;
        }
    } else {
        // TODO: These parameters are specific to the MW sandbox so we need to
        // store them in database?
        std::array<std::string, 3> excludedStems = {{"/.sbtools/", "/derived/", "toolbox_cache-glnxa64"}};
        std::array<std::string, 1> excludedExtensions = {{".p"}};
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

    // Display input parameters if verbose flag is on.
    if (params.Verbose) {
        params.disp();
    }

    // If help command is specified then we won't search for edited files.
    if (!params.Help) {
        // TODO: Use async programming model for this code segment.
        auto data = getEditedFiles(params);           // Get the list of edited files.
        auto lookupTable = loadDatabase(params);      // Load the lookup table from the database
        auto editedFiles = filter(lookupTable, data); // Filter edited file list
        print(params, editedFiles);
    }

    return 0;
}
