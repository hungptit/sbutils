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

#include <cereal/archives/binary.hpp>

int main(int argc, char *argv[]) {
    Tools::InputArgumentParser params(argc, argv);
    if (params.Verbose) {
        params.disp();
    }

    // Build file information database
    // Tools::BuildFileDatabase<Tools::Finder, Tools::BasicFileInfo> fSearch;
Tools::BuildFileDatabase<Tools::Finder, std::string> fSearch;
    for (const auto &val : params.Folders) {
        fSearch.search(val);
    }

Tools::disp(fSearch.getData(), "");

    // fSearch.disp<cereal::JSONOutputArchive>();
    // auto data = fSearch.getData();
    // Tools::disp<cereal::JSONOutputArchive, decltype(data)>(data);

    // Write results to database.
    Tools::Writer writer(params.Database);
    // writer.write(fSearch.getData());

    return 0;
}
