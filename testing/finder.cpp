#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <tuple>
#include "boost/filesystem.hpp"
#include "utils/Utils.hpp"
#include "boost/program_options.hpp"
#include "InputArgumentParser.hpp"

int main(int argc, char *argv[]) {
<<<<<<< HEAD
    Tools::InputArgumentParser parser(argc, argv);
    parser.disp();
    Tools::FindAllFiles<Tools::FindFiles> finder;
    for (const auto &val : parser.Folders) {
        finder.search(val);
    }
    Tools::Writer writer(parser.Database);
    writer.write(finder.getData());
    return EXIT_SUCCESS;
=======
    Tools::InputArgumentParser params(argc, argv);
    if (params.Verbose) {
        params.disp();
    }

    // Build file information database
    Tools::Finder fSearch;
    for (const auto &val : params.Folders) {
        fSearch.search(val);
    }

    // Create the database if desired.
    if (!params.Database.empty()) {
        Tools::Writer writer(params.Database);
        writer.write(fSearch.getData());
    }

    std::cout << Json::toJSON("Test", "Test");

    return 0;
>>>>>>> 7bc7a48c954473e5a9d0bc6fbcabf54c38397a13
}
