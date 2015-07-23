#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <tuple>
#include <unordered_map>

#define BOOST_THREAD_VERSION 4
#include "boost/config.hpp"
#include "boost/thread.hpp"
#include "boost/thread/future.hpp"

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

template <typename SearchAlg, typename Map> class Finder {
  public:
    typedef std::vector<typename Map::mapped_type> Container;

    Finder(Tools::InputArgumentParser &params, size_t maxlen = 1500000)
        : Params(params), Alg(Params.Extensions), MaxLen(maxlen) {
        ExcludedStrings = {{"/.sbtools/", "/derived/", "toolbox_cache-glnxa64", "~"}};
        ExcludedExtensions = {{".p", ".so", ".dbg"}};
    };

    void find() {
        for (const auto &aFolder : Params.Folders) {
            Alg.search(boost::filesystem::canonical(aFolder));
        }

        if (Params.Verbose) {
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
        std::string dataFile;
        if (Params.Database.empty()) {
            auto const sandbox = Tools::getSandboxRoot(Params.Folders[0]);
            if (!sandbox.empty()) {
                dataFile = (boost::filesystem::path(Tools::FileDatabaseInfo::Database)).string();
            }
        } else {
            dataFile = Params.Database;
        }

        if (dataFile.empty()) {
            std::cout << "Could not find the edited file database. All editable files in the given folders "
                         "will be listed!"
                      << std::endl;
            return;
        }

        Tools::Reader reader(dataFile);
        auto allKeys = reader.keys();

        if (Params.Verbose) {
            std::cout << "All keys in the " << Params.Database << " database: " << std::endl;
            for (auto const &info : allKeys) {
                std::cout << info << "\n";
            }
        }

        auto aKey = Tools::findParent(allKeys, Params.Folders[0]);
        auto const &results = reader.read(aKey);
        if (!results.empty()) {
            std::istringstream is(results);
            Tools::load<Tools::DefaultIArchive, decltype(database)>(database, is);
        }

        if (Params.Verbose) {
            std::cout << "Number of files in edited database: " << database.size() << std::endl;
        }

        // Reserve the space. Will need to adjust this parameter based on the number of files in the
        // sandbox.
        LookupTable.reserve(database.size());
        for (auto const &item : database) {
            auto aKey = std::get<0>(item);
            LookupTable.emplace(aKey, item);
        }

        if (Params.Verbose) {
            std::cout << "Look up table size: " << LookupTable.size() << std::endl;
        }
    }

    void get() {
        auto const data = Alg.getData();
        if (Params.Verbose) {
            std::cout << "Edited files: " << data.size() << std::endl;
        }
        for (const auto &anEditedFile : data) {
            auto aKey = std::get<0>(anEditedFile);
            auto aFile = LookupTable.find(aKey);
            if ((aFile == LookupTable.end()) || (anEditedFile != aFile->second)) {
                // If we could not find a given key in the database or the value
                // associated with that key is the the same with the current value
                // then we need to record this file.
                EditedFiles.emplace_back(anEditedFile);
            }
        }
    }
    
    // template <typename Iterator>
    // Container filter(Iterator first, Iterator last) {
    //     Container results;
    //     for (auto val = first; val != last; ++val) {
    //         bool isExcluded = false;
    //         auto aPath = std::get<0>(*val);
    //         isExcluded =
    //             std::any_of(ExcludedExtensions.begin(), ExcludedExtensions.end(),
    //                         [=](const std::string &extStr) { return extStr == std::get<2>(*val); }) ||
    //             std::any_of(ExcludedStrings.begin(), ExcludedStrings.end(), [=](const std::string &extStr) {
    //                 return aPath.find(extStr) != std::string::npos;
    //             });

    //         if (!isExcluded) {
    //             results.emplace_back(val);
    //         }
    //     }
    //     return results;
    // }

        void disp() {
        if (Params.Verbose) {
            std::cout << "Edited files: " << std::endl;
            for (const auto &val : EditedFiles) {
                std::cout << val << std::endl;
            }
        } else {
            std::array<std::string, 4> excludedStems = {
                {"/.sbtools/", "/derived/", "toolbox_cache-glnxa64", "~"}};
            std::array<std::string, 3> excludedExtensions = {{".p", ".so", ".dbg"}};

            // TODO: Improve this algorithm using thread.
            // Container results = filter(EditedFiles.begin(), EditedFiles.end());

            // TODO: Refine this algorithm
            Container results;
            for (const auto &val : EditedFiles) {
                bool isExcluded = false;
                auto aPath = std::get<0>(val);
                isExcluded =
                    std::any_of(excludedExtensions.begin(), excludedExtensions.end(),
                                [=](const std::string &extStr) { return extStr == std::get<2>(val); }) ||
                    std::any_of(excludedStems.begin(), excludedStems.end(), [=](const std::string &extStr) {
                        return aPath.find(extStr) != std::string::npos;
                    });

                if (!isExcluded) {
                    results.emplace_back(val);
                }
            }

            // Display results
            for (auto const val : results) {
                std::cout << std::get<0>(val) << std::endl;
            }
        }
    }

  private:
    Tools::InputArgumentParser Params;
    SearchAlg Alg;
    size_t MaxLen;
    Map LookupTable;
    Container EditedFiles;
    std::array<std::string, 4> ExcludedStrings;
    std::array<std::string, 4> ExcludedExtensions;
};

int main(int argc, char *argv[]) {
    typedef std::unordered_map<std::string, Tools::EditedFileInfo> Map;
    typedef Tools::FindEditedFiles<Tools::Finder> SearchAlg;

    Tools::InputArgumentParser params(argc, argv);
    if (params.Verbose) {
        params.disp();
    }

    if (!params.Help) {
        typedef Finder<SearchAlg, Map> FindEditedFiles;
        Finder<SearchAlg, Map> searchAlg(params);

        // Launch read and find tasks in two async threads.
        boost::future<void> readThread = boost::async(std::bind(&FindEditedFiles::read, &searchAlg));
        boost::future<void> findThread = boost::async(std::bind(&FindEditedFiles::find, &searchAlg));

        readThread.wait();
        findThread.wait();

        readThread.get();
        findThread.get();

        // Get the list of edited files then print out the results.
        searchAlg.get();
        searchAlg.disp();
    }
    return 0;
}
