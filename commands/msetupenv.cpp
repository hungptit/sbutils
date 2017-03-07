#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"
#include <array>
#include <cstdlib>
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
    std::string user = std::getenv("USER");
    std::string homeFolder = std::getenv("HOME");
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
    auto getAbsolutePathObj = [&rootFolder]() -> std::string {
        path aPath(rootFolder);
        if (!boost::filesystem::exists(aPath)) {
            boost::filesystem::create_directory(aPath);
        }
        return boost::filesystem::canonical(aPath).string();
    };
    rootFolder = getAbsolutePathObj();

    // bool verbose = vm.count("verbose");

    fmt::MemoryWriter writer;

    // Setup environment variables and display results.

    fmt::print("{}", writer.str());
    std::string varName;
    std::string value;
    int override = 1;

    // P4CLIENT
    varName = "P4CLIENT";
    writer << "export P4CLIENT=" << client << "\n";

    // P4PORT
    varName = "P4PORT";
    writer << "export P4PORT="
           << "perforce.athenahealth.com:1666\n";

    // P4_HOME
    varName = "P4_HOME";
    setenv(varName.c_str(), rootFolder.c_str(), override);
    writer << "export P4_HOME=" << rootFolder << "\n";

    // Other required settings
    writer << "$PATH:$P4_HOME/intranet/bin:$P4_HOME/techops/coredev/"
           << ":$P4_HOME/techops/coredev/bin\n";

    writer << "export BRANCH_MAP_PATH_OVERRIDE=1\n";

    fmt::print("{}\n", writer.str());
    return EXIT_SUCCESS;
}
