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

#include <string>
#include <vector>
#include <map>
#include <ctime>
#include <sstream>

int main(int argc, char *argv[]) {
    typedef cereal::JSONOutputArchive OArchive;
    typedef cereal::JSONInputArchive IArchive;

    Tools::InputArgumentParser params(argc, argv);
    if (!params.Help) {
        // Build file information database
        Tools::Writer writer(params.Database);
        for (const auto &val : params.Folders) {
            // Search for files
            Tools::BuildFileDatabase<Tools::Finder, Tools::BasicFileInfo> fSearch;
            fSearch.search(val);

            // Serialized file information to string
            std::ostringstream os;
            auto data = fSearch.getData();
            Tools::save<OArchive, decltype(data)>(data, os);
            const auto value = os.str();
            const auto key = val;

            // Write searched info to database.
            writer.write(key, value);

            // Display the information if the verbose flag is set.
            if (params.Verbose) {
                for (auto const &val : data) {
                    std::cout << val << "\n";
                }
            }
        }
    }

    // {
    //     decltype(data) test_data;
    //     std::istringstream is(os.str());
    //     Tools::load<IArchive, decltype(test_data)>(test_data, is);
    //     std::cout << os.str().size() << std::endl;
    //     for (auto val : test_data) {
    //         // std::cout << val << std::endl;
    //     }
    // }

    return 0;
}
