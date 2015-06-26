#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <tuple>
#include "boost/filesystem.hpp"
#include "utils/Utils.hpp"
#include "utils/LevelDBIO.hpp"

#include "boost/program_options.hpp"

int main(int argc, char *argv[]) {
    using namespace boost;
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    desc.add_options()("help,h", "finder - Find all files of a given folder and .")("verbose,v", "Display all data.")(
        "database,o", po::value<std::string>(), "An output database.");

    po::positional_options_description p;
    p.add("database", -1);
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << "Usage: viewer [options]\n";
        std::cout << desc;
        std::cout << "Examples:" << std::endl;
        std::cout << "\t viewer testdata" << std::endl;
        return 0;
    }

    std::string database;

    if (vm.count("database")) {
        database = vm["database"].as<std::string>();
    } else {
        database = "data";
    }

    // Read the database
    Tools::Reader reader(database);
    reader.read();
    
    return 0;
}
