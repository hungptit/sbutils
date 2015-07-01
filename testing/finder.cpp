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
    Tools::Writer writer(params.Database);
    for (const auto &val : params.Folders) {
        // Search for files
        Tools::BuildFileDatabase<Tools::Finder, Tools::BasicFileInfo> fSearch;
        fSearch.search(val);

        // Serialized file information to string
        std::ostringstream os;
        auto data = fSearch.getData();
        Tools::save<Tools::OArchive, decltype(data)>(data, os);
        const auto value = os.str();
        const auto key = val;

        // Write searched info to database.
        writer.write(key, value);
    }

    return 0;
}
