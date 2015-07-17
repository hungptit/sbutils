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

typedef Tools::DefaultOArchive OArchive;
typedef Tools::DefaultIArchive IArchive;

void createDatabase(const std::string &dataFile, const std::vector<std::string> &folders, bool verbose = false) {
    Tools::Writer writer(dataFile);
    for (const auto &aFolder : folders) {
        const std::string aPath = Tools::getAbslutePath(aFolder);
        // Search for files
        Tools::BuildFileDatabase<Tools::Finder, Tools::EditedFileInfo> fSearch;
        fSearch.search(aPath);
        auto data = fSearch.getData();

        // Serialized file information to string
        std::ostringstream os;
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
}

int main(int argc, char *argv[]) {
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

    // Create the edited file database.
    createDatabase(dataFile, folders, verbose);
    std::cout << "Edited file database: " << dataFile << std::endl;
    return 0;
}
