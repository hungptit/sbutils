#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"
#include <array>
#include <fstream>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

#include "fmt/format.h"

int main(int argc, char *argv[]) {
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");

    using path = boost::filesystem::path;

    std::string branch;
    std::string subBranch;
    std::string rootFolder;
    std::string client;

    std::string defaultClient = std::getenv("USER");
    std::string defaultRootFolder = (path(std::getenv("HOME")) / path("p4")).string();

    // clang-format off
    desc.add_options()
        ("help,h", "Print this help")
        ("verbose,v", "Display searched data.")
		("root-folder,s", po::value<std::string>(&rootFolder)->default_value(defaultRootFolder), "P4 root folder.")
		("client,c", po::value<std::string>(&client)->default_value(defaultClient), "P4 client.")
		("branch,b", po::value<std::string>(&branch), "P4 branch.")
		("sub-branch,u", po::value<std::string>(&subBranch)->default_value("anet/features/plaw/"), "P4 subbranch.");
    // clang-format on

    po::positional_options_description p;
    p.add("root-folder", -1);
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc;
        return EXIT_SUCCESS;
    }

	// Normalize root folder path.
    rootFolder = utils::getAbsolutePath(rootFolder);

    bool verbose = vm.count("verbose");

    fmt::MemoryWriter writer;

	// Setup environment variables and display results.

    fmt::print("{}", writer.str());

    return EXIT_SUCCESS;
}
