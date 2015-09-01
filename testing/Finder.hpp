#ifndef Finder_hpp_
#define Finder_hpp_

#include <string>
#include <tuple>
#include "cppformat/format.h"

enum {
    VERBOSE,
    DATAFILE,
    FOLDERS,
    STEMS,
    EXTENSIONS,
    SEARCHSTRINGS,
    REGEXPATTERNS
};

namespace {
    template <typename InputArguments>
    void printKeys(InputArguments &params, std::vector<std::string> &keys) {
        if (std::get<VERBOSE>(params)) {
            std::cout << "All keys in the " << std::get<DATAFILE>(params)
                      << " database: " << std::endl;
            for (auto const &info : keys) {
                std::cout << info << "\n";
            }
        }
    }
}

template <typename SearchAlg, typename Map, typename InputArguments>
class Finder {
  public:
    typedef std::vector<typename Map::mapped_type> Container;

    Finder(InputArguments &params, size_t maxlen = 1500000)
        : Params(params), Alg(std::get<EXTENSIONS>(Params)), MaxLen(maxlen),
          ExcludedStrings(
              {{"/.sbtools/", "/derived/", "toolbox_cache-glnxa64", "~"}}),
          ExcludedExtensions({{".p", ".so", ".dbg"}}){};

    void find() {
        auto folders = std::get<FOLDERS>(Params);
        if (folders.empty()) {
            Alg.search(boost::filesystem::canonical(
                boost::filesystem::current_path()));
        } else {
            for (const auto &aFolder : folders) {
                Alg.search(boost::filesystem::canonical(aFolder));
            }
        }

        if (std::get<VERBOSE>(Params)) {
            auto data = Alg.getData();
            std::cout << "==== Search results ====\n";
            std::cout << "Number of files: " << data.size() << '\n';
            for (auto const &info : data) {
                std::cout << std::get<0>(info) << "\n";
            }
        }
    }

    void read() {
        Container database;
        auto const dataFile = std::get<DATAFILE>(Params);
        bool verbose = std::get<VERBOSE>(Params);

        if (dataFile.empty()) {
            std::cout << "Could not find the edited file database. All "
                         "editable files in the given folders "
                         "will be listed!"
                      << std::endl;
            return;
        }

        // Read all keys from the database
        Tools::Reader reader(dataFile);
        auto allKeys = reader.keys();
        printKeys(Params, allKeys);

        // Find out which keys will be loaded.
        auto folders = std::get<FOLDERS>(Params);
        std::set<std::string> keys;
        bool loadAllKeys = false;
        if (folders.empty()) {
            loadAllKeys = true;
        } else {
            for (auto aPath : folders) {
                auto aKey = Tools::findParent(allKeys, aPath);
                if (aKey.empty()) {
                    loadAllKeys = true; // Cannot find a current key. We need to load all keys.
                    break;
                } else {
                    keys.insert(aKey);
                }
            }
        }

        if (loadAllKeys) {
            for (auto const &aKey : allKeys) {
                keys.insert(aKey);
            }
        }

        // Load all data from the database. This piece of code can be moved out.
        for (auto const aKey : keys) {
            auto const results = reader.read(aKey);
            if (!results.empty()) {
                decltype(database) data;
                std::istringstream is(results);
                Tools::load<Tools::DefaultIArchive, decltype(data)>(data, is);
                std::move(data.begin(), data.end(),
                          std::back_inserter(database));
            }
        }

        if (verbose) {
            std::cout << "Number of files in edited database: "
                      << database.size() << std::endl;
        }

        // Build the lookup table.
        LookupTable.reserve(database.size());
        for (auto const &item : database) {
            auto aKey = std::get<0>(item);
            LookupTable.emplace(aKey, item);
        }

        // Print out the verbose information.
        if (verbose) {
            std::cout << "Look up table size: " << LookupTable.size()
                      << std::endl;
        }
    }

    size_t filter() {
        auto const data = Alg.getData();
        if (std::get<VERBOSE>(Params)) {
            std::cout << "Edited files: " << data.size() << std::endl;
        }

        size_t counter = 0;
        for (const auto &anEditedFile : data) {
            auto aKey = std::get<0>(anEditedFile);
            auto aFile = LookupTable.find(aKey);
            if ((aFile == LookupTable.end()) ||
                (anEditedFile != aFile->second)) {
                counter ++;
                // If we could not find a given key in the database or the value
                // associated with that key is the the same with the current
                // value then we need to record this file.
                const std::array<std::string, 4> excludedStems = {
                    {"/.sbtools/", "/derived/", "toolbox_cache-glnxa64", "~"}};
                const std::array<std::string, 3> excludedExtensions = {
                    {".p", ".so", ".dbg"}};

                bool isExcluded = false;
                auto aPath = std::get<0>(anEditedFile);
                isExcluded =
                    std::any_of(excludedExtensions.begin(),
                                excludedExtensions.end(),
                                [=](const std::string &extStr) {
                                    return extStr == std::get<2>(anEditedFile);
                                }) ||
                    std::any_of(excludedStems.begin(), excludedStems.end(),
                                [=](const std::string &extStr) {
                                    return aPath.find(extStr) !=
                                           std::string::npos;
                                });

                if (!isExcluded) {
                    EditedFiles.emplace_back(anEditedFile);
                }
            }
        }
        return counter;
    }

    void disp() {
        if (std::get<VERBOSE>(Params)) {
            std::cout << "Edited files: " << std::endl;
            for (const auto &val : EditedFiles) {
                fmt::print("{}\n", val);
            }
        } else {
            for (auto const val : EditedFiles) {
                fmt::print("{}\n", std::get<0>(val));
            }
        }
    }

    Container &getEditedFiles() { return EditedFiles; }

  private:
    InputArguments Params;
    SearchAlg Alg;
    size_t MaxLen;
    Map LookupTable;
    Container EditedFiles;
    std::array<std::string, 4> ExcludedStrings;
    std::array<std::string, 4> ExcludedExtensions;
};
#endif
