#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <tuple>
#include "boost/filesystem.hpp"
#include "utils/Utils.hpp"
#include "utils/FindUtils.hpp"
#include "utils/LevelDBIO.hpp"
#include "boost/program_options.hpp"
#include "InputArgumentParser.hpp"

typedef Tools::DefaultOArchive OArchive;
typedef Tools::DefaultIArchive IArchive;

void createDatabase(const std::string & dataFile, const std::vector<std::string> & folders, bool verbose = false) {
    Tools::Writer writer(dataFile);
    for (const auto &aFolder : folders) {
        const std::string aPath = Tools::getAbslutePath(aFolder);
        // Search for files
        Tools::BuildFileDatabase<Tools::Finder, Tools::EditedFileInfo> fSearch;
        fSearch.search(aPath);
        auto data = fSearch.getData();
            
        // Serialized file information to string
        std::ostringstream os;
        Tools::save<OArchive, decltype(data)>(data, os);
        const auto value = os.str();

        // Write searched info to database.
        writer.write(aPath, value);

        // Display the information if the verbose flag is set.
        if (verbose) {
            for (auto const &info : data) {
                std::cout << info << "\n";
            }
        }
    }    
}


int main(int argc, char *argv[]) {
    Tools::InputArgumentParser params(argc, argv);
    if (!params.Help) {
        auto dataFile = params.Database;
        if (dataFile.empty()) {
            dataFile = (Tools::getSandboxRoot(dataFile) / boost::filesystem::path(".sbtools") / boost::filesystem::path("database")).string();
        }
        createDatabase(params.Database, params.Folders, params.Verbose);
    }
    return 0;
}
