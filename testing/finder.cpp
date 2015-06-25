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
    Tools::InputArgumentParser params(argc, argv);
    if (params.Verbose) {
        params.disp();
    }

    // Build file information database
    Tools::BuildFileDatabase<Tools::Finder, Tools::BasicFileInfo> fSearch;
    for (const auto &val : params.Folders) {
        fSearch.search(val);
    }
    
    // IO data format
    typedef cereal::JSONOutputArchive OArchive;
    typedef cereal::JSONInputArchive IArchive;

    std::ostringstream os;
    auto data = fSearch.getData();

    {
        Tools::save<OArchive, decltype(data)>(data, os);
        std::cout << os.str().size() << std::endl;
    }

    {
        decltype(data) test_data;
        std::istringstream is(os.str());
        Tools::load<IArchive, decltype(test_data)>(test_data, is);
        std::cout << os.str().size() << std::endl;
        for (auto val : test_data) {
            std::cout << val << std::endl;
        }
    }

    // Write results to database.
    // Tools::Writer writer(params.Database);
    // writer.write(fSearch.getData());

    return 0;
}
