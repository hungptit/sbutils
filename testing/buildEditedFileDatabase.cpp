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

template <typename OArchive, typename Container>
void serialize(Tools::Writer &writer, const std::string &key, Container &data) {
    std::ostringstream os;
    Tools::save<OArchive, decltype(data)>(data, os);
    const auto value = os.str();
    writer.write(key, value);
}

std::vector<Tools::EditedFileInfo>
createFileInfo(std::vector<boost::filesystem::path> &files) {
    std::vector<Tools::EditedFileInfo> data;
    for (auto const &aPath : files) {
        boost::system::error_code ec;
        auto const fs = boost::filesystem::status(aPath, ec);
        if (fs.type() != boost::filesystem::status_error) {
            data.emplace_back(
                std::make_tuple(aPath.string(), aPath.stem().string(),
                                aPath.extension().string(), fs.permissions(),
                                boost::filesystem::last_write_time(aPath)));
        }
    }
    return data;
}

void createFolderDatabase(Tools::Writer &writer,
                          std::vector<boost::filesystem::path> &folders,
                          bool verbose) {
    for (const auto &aFolder : folders) {
        const auto aPath = boost::filesystem::canonical(aFolder);
        std::cout << "Build file database for " << aFolder << "\n";

        // Search for files
        Tools::BuildFileDatabase<Tools::Finder, Tools::EditedFileInfo> fSearch;
        fSearch.search(aPath);
        auto data = fSearch.getData();

        // Serialized file information to string then write it to database
        // std::ostringstream os;
        // Tools::save<OArchive, decltype(data)>(data, os);
        // const auto value = os.str();
        // writer.write(aPath.string(), value);
        serialize<OArchive, decltype(data)>(writer, aPath.string(), data);

        // Display the information if the verbose flag is set.
        if (verbose) {
            for (auto const &info : data) {
                std::cout << info << "\n";
            }
        }
    }
}

void createDatabase(const std::string &dataFile,
                    const std::vector<std::string> &folders,
                    const int exploreLevel, bool verbose = false) {
    Tools::Writer writer(dataFile);
    if (folders.empty()) {
        auto results = Tools::exploreFolders(
            exploreLevel,
            boost::filesystem::canonical(boost::filesystem::current_path()));
        createFolderDatabase(writer, std::get<0>(results), verbose);

        // Now write all files to a unique key.
        auto data = createFileInfo(std::get<0>(results));
        serialize<OArchive, decltype(data)>(writer, "files_at_root_level", data);
    } else {
        std::vector<boost::filesystem::path> data;
        for (auto &val : folders) {
            data.emplace_back(boost::filesystem::path(val));
        }
        createFolderDatabase(writer, data, verbose);
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
    po::store(
        po::command_line_parser(argc, argv).options(desc).positional(p).run(),
        vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << "Usage: buildEditedFileDatabase [options]\n";
        std::cout << desc;
        std::cout << "Examples:" << std::endl;
        std::cout << "\t finder ./ -d testdata." << std::endl;
        return 0;
    }

    bool verbose = false;
    if (vm.count("verbose")) {
        verbose = true;
    }

    std::vector<std::string> folders;

    if (vm.count("folders")) {
        folders = vm["folders"].as<std::vector<std::string>>();
    }

    std::string dataFile;
    if (vm.count("database")) {
        dataFile = vm["database"].as<std::string>();
    } else {
        dataFile = (boost::filesystem::path(Tools::FileDatabaseInfo::Database))
                       .string();
    }

    // Create the edited file database.
    createDatabase(dataFile, folders, 2, verbose);
    std::cout << "Edited file database: " << dataFile << std::endl;
    return 0;
}
