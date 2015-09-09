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
#include "cppformat/format.h"

#include <string>
#include <vector>
#include <map>
#include <ctime>
#include <sstream>

int main(int argc, char *argv[]) {
    typedef cereal::JSONOutputArchive OArchive;
    typedef cereal::JSONInputArchive IArchive;

    using namespace boost;
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");

    // clang-format off
    desc.add_options()              
        ("help,h", "Print this help")
        ("verbose,v", "Display searched data.")
        ("use-relative-path,r", "Use relative path.")
        ("toJSON,j", po::value<std::string>(), "Output results in a JSON file.")
        ("folders,f", po::value<std::vector<std::string>>(), "Search folders.")
        ("file-stems,s", po::value<std::vector<std::string>>(), "File stems.")
        ("extensions,e", po::value<std::vector<std::string>>(), "File extensions.")
        ("search-strings,t", po::value<std::vector<std::string>>(), "File extensions.");
    // clang-format on

    po::positional_options_description p;
    p.add("folders", -1);
    po::variables_map vm;
    po::store(
        po::command_line_parser(argc, argv).options(desc).positional(p).run(),
        vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc;
        std::cout << "Examples:" << std::endl;
        std::cout << "\t findEditedFiles ./ -d .database" << std::endl;
        return EXIT_SUCCESS;
    }

    auto verbose = false;
    if (vm.count("verbose")) {
        verbose = true;
    }

    auto useRelativePath = false;
    if (vm.count("use-relative-path")) {
        useRelativePath = true;
    }

    std::string jsonFile;
    if (vm.count("toJSON")) {
        jsonFile = vm["toJSON"].as<std::string>();
    }

    boost::system::error_code errcode;
    std::vector<std::string> folders;
    if (vm.count("folders")) {
        for (auto item : vm["folders"].as<std::vector<std::string>>()) {
            folders.emplace_back(boost::filesystem::path(item).string());
        }
    }

    std::vector<std::string> stems;
    if (vm.count("file-stems")) {
        stems = vm["file-stems"].as<std::vector<std::string>>();
    }

    std::vector<std::string> extensions;
    if (vm.count("extensions")) {
        extensions = vm["extensions"].as<std::vector<std::string>>();
    }

    std::vector<std::string> searchStrings;
    if (vm.count("search-strings")) {
        searchStrings = vm["search-strings"].as<std::vector<std::string>>();
    }

    // Search for files in the given folders.
    Tools::FindFiles<Tools::Finder> searchAlg(extensions);
    
    for (auto &val : folders) {
        searchAlg.search(val);
    }
    auto data = searchAlg.getData();

    if (verbose) {
        fmt::print("Search folders:\n");
        for (const auto &val : folders) {
            fmt::print("{}\n", val);
        }
        fmt::print("Number of files: {}\n", data.size());
        
        std::for_each(data.begin(), data.end(), [] (auto const & val) {
            fmt::print("({0}, {1}, {2})\n", std::get<0>(val), std::get<1>(val),
                       std::get<2>(val));
        });
    }

    if (!jsonFile.empty()) {
        std::ostringstream os;
        Tools::save<OArchive, decltype(data)>(data, os);
        std::ofstream myfile(jsonFile);
        myfile << os.str() << std::endl;
    }      

    return 0;
}
