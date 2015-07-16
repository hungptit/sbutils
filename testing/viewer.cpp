#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <tuple>
#include <unordered_map>

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

int main(int argc, char *argv[]) {
    using namespace boost;
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    desc.add_options()     
      ("help,h", "finder - Find all files of a given folder and .")
      ("verbose,v", "Display all data.")
      ("database,d", po::value<std::string>(), "Edited file database.")
      ("extensions,e", po::value<std::string>(), "File extension");
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
    auto allKeys = reader.keys();
    for (auto &aKey : allKeys) {
        std::vector<Tools::EditedFileInfo> test_data;
        auto buffer = reader.read(aKey);
        std::istringstream is(buffer);
        Tools::load<Tools::DefaultIArchive, decltype(test_data)>(test_data, is);
        std::cout << buffer.size() << std::endl;
        for (auto val : test_data) {
            std::cout << std::get<0>(val) << std::endl;
        }
    }

    return 0;
}
