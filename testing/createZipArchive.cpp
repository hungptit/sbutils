#include <fstream>
#include <iostream>
#include "boost/program_options.hpp"
#include "boost/lexical_cast.hpp"
#include "utils/Utils.hpp"
#include "utils/Process.hpp"

int main(int ac, char *av[]) {
    using namespace boost;
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    desc.add_options()
      ("help,h", "createZipArchive - Create zip file for each git repository.")
      ("verbose,v", "Display input arguments.")
      ("folder,d", po::value<std::vector<std::string>>(), "A git repository.")      
      ("zip-file,o", po::value<std::string>(), "An output zip file.");

    po::positional_options_description p;
    p.add("folder", -1);
    po::variables_map vm;
    po::store(po::command_line_parser(ac, av).options(desc).positional(p).run(),
              vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << "Usage: createZipArchive [options]\n";
        std::cout << desc;
        std::cout << "Examples:" << std::endl;
        std::cout
            << "\t createZipArchive -d utils.git - Create a utils.zip file."
            << std::endl;
        std::cout << "\t createZipArchive utils.git - Create a utils.zip file."
                  << std::endl;
        std::cout << "\t createZipArchive - Create a utils.zip file for al git "
                     "repositories." << std::endl;
        return false;
    }

    std::vector<std::string> folders;
    boost::filesystem::path aZipFile;

    if (vm.count("folder")) {
        folders = vm["folder"].as<std::vector<std::string>>();
    } else {
        folders.emplace_back("3p.git");
        folders.emplace_back("utils.git");
        folders.emplace_back("sbtools.git");
        folders.emplace_back("hdf5.git");
        folders.emplace_back("testing.git");
        folders.emplace_back("matlab-json.git");
        folders.emplace_back("matlab-sqlite3.git");
    }

    const auto firstFolder = boost::filesystem::path(folders[0]);
    const auto parentPath =
        boost::filesystem::canonical(firstFolder).parent_path();
    if (vm.count("zip-file")) {
        aZipFile = parentPath /
                   boost::filesystem::path(vm["zip-file"].as<std::string>());
    } else {
        aZipFile = parentPath / boost::filesystem::path("data.zip");
    }

    Tools::disp(folders, "Added folders: ");
    std::cout << " to " + aZipFile.string() << std::endl;

    // Create a zip file
    const std::string command("/usr/bin/zip");
    std::vector<std::string> args = {"-r", "-9", aZipFile.string()};
    for (const auto &val : folders) {
        args.emplace_back(val);
    }
    const std::string output = Tools::run(command, args);
    if (vm.count("verbose")) {
        std::cout << output << std::endl;
    }

    return 0;
}
