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
        ("extensions,e", po::value<std::vector<std::string>>(), "Search extensions.");

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
    std::vector<std::string> folders = {"./"};
    std::string database ;
    std::vector<std::string> searchExtensions = {std::string(".m"), ".mat", ".slx", ".cpp", ".hpp"};
    
    if (vm.count("folder")) {
        folders = vm["folder"].as<std::vector<std::string>>();
    }

    if (vm.count("extensions")) {
        searchExtensions = vm["extensions"].as<std::vector<std::string>>();
    }   

    // Search for an edited files in given folders.
    Tools::FindEditedFiles fSearch;
    for (const auto & val : folders) {
        fSearch.search(val, searchExtensions);
    }
    
    fSearch.print();
    return 0;
}
