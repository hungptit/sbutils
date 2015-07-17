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

int main(int argc, char *argv[]) {
    typedef Tools::DefaultOArchive OArchive;
    typedef Tools::DefaultIArchive IArchive;
    using namespace boost;
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");

    // clang-format off
    desc.add_options()
        ("help,h", "Print this help")
        ("verbose,v", "Display searched data.")
        ("folders,f", po::value<std::vector<std::string>>(), "Search folders.")
        ("database,d", po::value<std::string>(), "File database.");
    // clang-format on

    po::positional_options_description p;
    p.add("folders", -1);
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << "Usage: buildEditedFileDatabase [options]\n";
        std::cout << desc;
        std::cout << "Examples:" << std::endl;
        std::cout << "\t finder ./ -d testdata." << std::endl;
        return 0;
    }

    bool verbose = false;
    if (vm.count("verbose")) { verbose = true;}

    std::vector<std::string> folders;

    if (vm.count("folders")) {
        folders = vm["folders"].as<std::vector<std::string>>();
    } else {
        folders.emplace_back(boost::filesystem::current_path().string());
    }

    std::string dataFile;
    if (vm.count("database")) {
        dataFile = vm["database"].as<std::string>();
    } else {
        dataFile = (boost::filesystem::path(Tools::FileDatabaseInfo::Database)).string();
    }

    // Build file information database
    Tools::Writer writer(dataFile);
    for (const auto &aFolder : folders) {
        // Search for files
        const std::string aPath = Tools::getAbslutePath(aFolder);
        Tools::BuildFileDatabase<Tools::Finder, Tools::BasicFileInfo> fSearch;
        fSearch.search(aPath);

        // Serialized file information to string
        std::ostringstream os;
        auto data = fSearch.getData();
        Tools::save<OArchive, decltype(data)>(data, os);
        const auto value = os.str();

        // Write searched info to database.
        writer.write(aPath, value);

        // Display the information if the verbose flag is set.
        if (verbose) {
            for (auto const &info : data) {
                std::cout << info << "\n";
            }
        }
    }
    return 0;
}
