#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"
#include <array>
#include <fstream>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

#include "fmt/format.h"
#include "sbutils/FileUtils.hpp"

using path = boost::filesystem::path;

std::string removeRedundantSlash(const path &aPath) {
  path results;
  path currentPath(aPath);
  while (!currentPath.empty()) {
    std::string aStem = sbutils::normalize_path(currentPath.stem().string());
    if (aStem != ".") {
      results = path(aStem) / results;
    }
    currentPath = currentPath.parent_path();
  }
  return results.string();
}

int main(int argc, char *argv[]) {
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");

    using path = boost::filesystem::path;

    std::string branch;
    std::string subBranch;
    std::string rootFolder;
    std::string client;

    std::string defaultClient = std::getenv("P4CLIENT");
    std::string defaultRootFolder = (path(std::getenv("P4_HOME"))).string();

    // clang-format off
    desc.add_options()
        ("help,h", "Print this help")
        ("verbose,v", "Display searched data.")
		("root-folder,s", po::value<std::string>(&rootFolder)->default_value(defaultRootFolder), "P4 root folder.")
		("client,c", po::value<std::string>(&client)->default_value(defaultClient), "P4 client.")
		("branch,b", po::value<std::string>(&branch), "P4 branch.")
		("sub-branch,u", po::value<std::string>(&subBranch)->default_value("anet/features/plaw"), "P4 subbranch.");
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

	// Normalize the root folder path
	auto getAbsolutePathObj = [&rootFolder]() -> std::string {
		path aPath(rootFolder);
		if (!boost::filesystem::exists(aPath)) {
			boost::filesystem::create_directory(aPath);
		}
		return boost::filesystem::canonical(aPath).string();
	};
    rootFolder = getAbsolutePathObj();

    bool verbose = vm.count("verbose");

    fmt::MemoryWriter writer;

    // clang-format off
    writer << "Client: " << client << "\n\n";
    writer << "Update: " << "\n\n";
    writer << "Access: " << "\n\n";
    writer << "Owner: " << std::getenv("USER") << "\n\n";
    writer << "Description: \n\tAutogenerated p4 client settings.\n\n";
    writer << "Root: " << rootFolder << "\n\n";
    writer << "AltRoots: " << rootFolder << "\n\n";
    writer << "Options: noallwrite clobber nocompress unlocked nomodtime rmdir\n\n";
    writer << "SubmitOptions: revertunchanged\n\n";
    writer << "LineEnd: local\n\n";
    writer << "View:\n";
    // clang-format on

    // Normalize all paths
    const std::string sourcePath = removeRedundantSlash(path("/depot") / path(subBranch) / path(branch));
    
    // Write out the setup for synced files and folders.
    writer << "\t/" << sourcePath << "/athenax/release/..."
           << " //" << client << "/athenax/release/...\n"
           << "\t/" << sourcePath << "/prod/..."
           << " //" << client << "/prod/..."
           << "\n"
           << "\t/" << sourcePath << "/techops/..."
           << " //" << client << "/techops/..."
           << "\n"
           << "\t-/" << sourcePath << "/prod/.generated/..."
           << " //" << client << "/prod/.generated/..."
           << "\n"
           << "\t-/" << sourcePath << "/prod/activexcontrols/..."
           << " //" << client << "/prod/activexcontrols/..."
           << "\n"
           << "\t-/" << sourcePath << "/prod/claimrules/..."
           << " //" << client << "/prod/claimrules/..."
           << "\n"
           << "\t-/" << sourcePath << "/prod/data/..."
           << " //" << client << "/prod/data/..."
           << "\n"
           << "\t-/" << sourcePath << "/prod/htdocs/appealforms/..."
           << " //" << client << "/prod/htdocs/appealforms/..."
           << "\n"
           << "\t-/" << sourcePath << "/prod/htdocs/billing/payersitenavigationguides/..."
           << " //" << client << "/prod/htdocs/billing/payersitenavigationguides/..."
           << "\n"
           << "\t-/" << sourcePath << "/prod/htdocs/interfaces/..."
           << " //" << client << "/prod/htdocs/static/interfaces/..."
           << "\n"
           << "\t-/" << sourcePath << "/prod/htdocs/payorforms/..."
           << " //" << client << "/prod/htdocs/static/payorforms/..."
           << "\n"
           << "\t-/" << sourcePath << "/prod/htdocs/postings/..."
           << " //" << client << "/prod/htdocs/postings/..."
           << "\n"
           << "\t-/" << sourcePath << "/prod/htdocs/static/*.exe"
           << " //" << client << "/prod/htdocs/static/*.exe"
           << "\n"
           << "\t-/" << sourcePath << "/prod/htdocs/static/*.zip"
           << " //" << client << "/prod/htdocs/static/*.zip"
           << "\n"
           << "\t-/" << sourcePath << "/prod/htdocs/static/thirdparty/artemis/..."
           << " //" << client << "/prod/htdocs/static/thirdparty/artemis/..."
           << "\n"
           << "\t-/" << sourcePath << "/prod/htdocs/static/claimrules/..."
           << " //" << client << "/prod/htdocs/static/claimrules/..."
           << "\n"
           << "\t-/" << sourcePath << "/prod/htdocs/static/clinicals/devices..."
           << " //" << client << "/prod/htdocs/static/clinicals/devices/..."
           << "\n"
           << "\t-/" << sourcePath << "/prod/htdocs/static/pagehelp/..."
           << " //" << client << "/prod/htdocs/static/pagehelp/..."
           << "\n"
           << "\t-/" << sourcePath << "/prod/htdocs/static/shared/executable..."
           << " //" << client << "/prod/htdocs/static/shared/executable..."
           << "\n"
           << "\t-/" << sourcePath << "/prod/mis/..."
           << " //" << client << "/prod/mis/..."
           << "\n"
           << "\t-/" << sourcePath << "/prod/mswindows/..."
           << " //" << client << "/prod/mswindow/..."
           << "\n"
           << "\t-/" << sourcePath << "/prod/ohelp_source/..."
           << " //" << client << "/prod/ohelp_source/..."
           << "\n"
           << "\t-/" << sourcePath << "/prod/patientcycle/sounds/..."
           << " //" << client << "/prod/patientcycle/sounds/..."
           << "\n"
           << "\t-/" << sourcePath << "/prod/perllib/Athena/PaperForm/PDF....pdf"
           << " //" << client << "/prod/perlib/Athena/PaperForms/PDF....pdf"
           << "\n"
           << "\t-/" << sourcePath << "/prod/scripts/app/clinicals/p4p/..."
           << " //" << client << "/prod/scripts/app/clinicals/p4p/..."
           << "\n"
           << "\t-/" << sourcePath << "/prod/sugarcrm/..."
           << " //" << client << "/prod/sugarcrm/..."
           << "\n"
           << "\t-/" << sourcePath << "/techops/*/dailydml/..."
           << " //" << client << "/techops/*/dailydml/..."
           << "\n"
           << "\t-/" << sourcePath << "/techops/*/dat/..."
           << " //" << client << "/techops/*/dat/..."
           << "\n"
           << "\t-/" << sourcePath << "/techops/*/imports/..."
           << " //" << client << "/techops/*/onetimejobs/..."
           << "\n"
           << "\t-/" << sourcePath << "/techops/*/oneoffscripts/..."
           << " //" << client << "/techops/*/oneoffscripts/..."
           << "\n"
           << "\t-/" << sourcePath << "/techops/*/onetimejobs/..."
           << " //" << client << "/techops/*/onetimejobs/..."
           << "\n"
           << "\t-/" << sourcePath << "/techops/dat/..."
           << " //" << client << "/techops/dat/..."
           << "\n"
           << "\t-/" << sourcePath << "/techops/mis/..."
           << " //" << client << "/techops/mis/..."
           << "\n"
           << "\t-/" << sourcePath << "/techops/sugar/..."
           << " //" << client << "/techops/sugar/..."
           << "\n";

    fmt::print("{}", writer.str());

    return EXIT_SUCCESS;
}
