#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "boost/filesystem.hpp"
#include "utils/Utils.hpp"
#include "boost/program_options.hpp"

int main(int argc, char *argv[]) {
    using namespace boost;
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");

    // clang-format off
    desc.add_options()
        ("help,h", "Print this help")
        ("verbose,v", "Display searched data.")
        ("files,f", po::value<std::vector<std::string>>(), "Files want to delete.")
        ("stems,s", po::value<std::string>(), "The stem of a file want to delete.")
        ("extensions,e", po::value<std::string>(), "The extension of a file want to delete.");
    // clang-format on

    po::positional_options_description p;
    p.add("files", -1);
    po::variables_map vm;
    po::store(
        po::command_line_parser(argc, argv).options(desc).positional(p).run(),
        vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << "Usage: deleteFiles [options]\n";
        std::cout << desc;
        std::cout << "Examples:" << std::endl;
        std::cout << "\t deleteFiles foo.txt test.txt" << std::endl;
        return 0;
    }

    bool verbose = false;
    if (vm.count("verbose")) {
        verbose = true;
    }

    std::vector<std::string> fileNames;
    if (vm.count("files")) {
        fileNames = vm["files"].as<std::vector<std::string>>();
    }

    std::string extensions;
    if (vm.count("extensions")) {
        extensions = vm["extensions"].as<std::string>();
    }

    std::string stems;
    if (vm.count("stems")) {
        stems = vm["stems"].as<std::string>();
    }

    // Delete files
    for (auto aFile : fileNames) {
        auto aPath = boost::filesystem::path(aFile);
        bool isOK =
            (stems.empty() || (stems == aPath.stem().string())) &&
            (extensions.empty() || (extensions == aPath.extension().string()));
        if (isOK) {
            if (boost::filesystem::exists(aPath)) {
                system::error_code errcode;
                boost::filesystem::remove(aPath, errcode);
                if (errcode == boost::system::errc::success) {
                    std::cout << "Cannot delete " << aPath << "\n";
                }
            }
            else {
                std::cout << "File " << aPath << " does not exist!\n";
            } 
        }
    }
}
