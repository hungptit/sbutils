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
    Tools::InputArgumentParser parser(argc, argv);
    parser.disp();
    Tools::FindAllFiles<Tools::FindFiles> finder;
    for (const auto &val : parser.Folders) {
        finder.search(val);
    }
    Tools::Writer writer(parser.Database);
    writer.write(finder.getData());
    return EXIT_SUCCESS;
}
