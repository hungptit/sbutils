#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <tuple>
#include "boost/filesystem.hpp"
#include "utils/Utils.hpp"
#include "boost/program_options.hpp"

int main (int argc, char * argv[]) {
    using namespace boost;
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "finder - Find all files of a given folder and .")
        ("verbose,v", "Display all data.")
        ("folder,d", po::value<std::vector<std::string>>(), "Search folders.")      
        ("database,o", po::value<std::string>(), "An output database.");

    po::positional_options_description p;
    p.add("folder", -1);
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(),
              vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << "Usage: finder [options]\n";
        std::cout << desc;
        std::cout << "Examples:" << std::endl;
        std::cout
            << "\t finder ./ -o testdata."
            << std::endl;
        return 0;
    }

    // Process input parameters
    std::vector<std::string> folders;
    std::string database ;

    if (vm.count("folder")) {
        folders = vm["folder"].as<std::vector<std::string>>();
    } else {
        folders.emplace_back("./");
    }

    if (vm.count("database")) {
        database = vm["database"].as<std::string>();
    } else {
        database = "data";
    }

    // Build file information database
    Tools::Finder fSearch;
    for (const auto & val : folders) {
        fSearch.search(val);
    }
    Tools::Writer writer(database);
    writer.write(fSearch.getData());    
    return 0;
}
