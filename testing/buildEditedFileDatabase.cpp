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

int main(int argc, char *argv[]) {
    typedef Tools::DefaultOArchive OArchive;
    typedef Tools::DefaultIArchive IArchive;
    Tools::InputArgumentParser params(argc, argv);
    if (!params.Help) {
        // Build file information database
        Tools::Writer writer(params.Database);
        for (const auto &aFolder : params.Folders) {
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
            if (params.Verbose) {
                for (auto const &info : data) {
                    std::cout << info << "\n";
                }
            }
        }
    }
    return 0;
}
