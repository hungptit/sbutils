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
		("sub-branch,c", po::value<std::string>(&subBranch)->default_value("anet/features/plaw/"), "P4 subbranch.");
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
		return aPath.string();
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

    // Write out the setup for synced files and folders.
    writer << "\t//depot/" << subBranch << branch << "/athenax/release/..."
           << " //" << client << "/athenax/release/..."
           << "\n"
           << "\t//depot/" << subBranch << branch << "/prod/..."
           << " //" << client << "/prod/..."
           << "\n"
           << "\t//depot/" << subBranch << branch << "/techops/..."
           << " //" << client << "/techops/..."
           << "\n"
           << "\t-//depot/" << subBranch << branch << "/prod/.generated/..."
           << " //" << client << "/prod/.generated/..."
           << "\n"
           << "\t-//depot/" << subBranch << branch << "/prod/activexcontrols/..."
           << " //" << client << "/prod/activexcontrols/..."
           << "\n"
           << "\t-//depot/" << subBranch << branch << "/prod/claimrules/..."
           << " //" << client << "/prod/claimrules/..."
           << "\n"
           << "\t-//depot/" << subBranch << branch << "/prod/data/..."
           << " //" << client << "/prod/data/..."
           << "\n"
           << "\t-//depot/" << subBranch << branch << "/prod/htdocs/appealforms/..."
           << " //" << client << "/prod/htdocs/appealforms/..."
           << "\n"
           << "\t-//depot/" << subBranch << branch
           << "/prod/htdocs/billing/payersitenavigationguides/..."
           << " //" << client << "/prod/htdocs/billing/payersitenavigationguides/..."
           << "\n"
           << "\t-//depot/" << subBranch << branch << "/prod/htdocs/interfaces/..."
           << " //" << client << "/prod/htdocs/static/interfaces/..."
           << "\n"
           << "\t-//depot/" << subBranch << branch << "/prod/htdocs/payorforms/..."
           << " //" << client << "/prod/htdocs/static/payorforms/..."
           << "\n"
           << "\t-//depot/" << subBranch << branch << "/prod/htdocs/postings/..."
           << " //" << client << "/prod/htdocs/postings/..."
           << "\n"
           << "\t-//depot/" << subBranch << branch << "/prod/htdocs/static/*.exe"
           << " //" << client << "/prod/htdocs/static/*.exe"
           << "\n"
           << "\t-//depot/" << subBranch << branch << "/prod/htdocs/static/*.zip"
           << " //" << client << "/prod/htdocs/static/*.zip"
           << "\n"
           << "\t-//depot/" << subBranch << branch
           << "/prod/htdocs/static/thirdparty/artemis/..."
           << " //" << client << "/prod/htdocs/static/thirdparty/artemis/..."
           << "\n"
           << "\t-//depot/" << subBranch << branch << "/prod/htdocs/static/claimrules/..."
           << " //" << client << "/prod/htdocs/static/claimrules/..."
           << "\n"
           << "\t-//depot/" << subBranch << branch
           << "/prod/htdocs/static/clinicals/devices..."
           << " //" << client << "/prod/htdocs/static/clinicals/devices/..."
           << "\n"
           << "\t-//depot/" << subBranch << branch << "/prod/htdocs/static/pagehelp/..."
           << " //" << client << "/prod/htdocs/static/pagehelp/..."
           << "\n"
           << "\t-//depot/" << subBranch << branch
           << "/prod/htdocs/static/shared/executable..."
           << " //" << client << "/prod/htdocs/static/shared/executable..."
           << "\n"
           << "\t-//depot/" << subBranch << branch << "/prod/mis/..."
           << " //" << client << "/prod/mis/..."
           << "\n"
           << "\t-//depot/" << subBranch << branch << "/prod/mswindows/..."
           << " //" << client << "/prod/mswindow/..."
           << "\n"
           << "\t-//depot/" << subBranch << branch << "/prod/ohelp_source/..."
           << " //" << client << "/prod/ohelp_source/..."
           << "\n"
           << "\t-//depot/" << subBranch << branch << "/prod/patientcycle/sounds/..."
           << " //" << client << "/prod/patientcycle/sounds/..."
           << "\n"
           << "\t-//depot/" << subBranch << branch
           << "/prod/perllib/Athena/PaperForm/PDF....pdf"
           << " //" << client << "/prod/perlib/Athena/PaperForms/PDF....pdf"
           << "\n"
           << "\t-//depot/" << subBranch << branch
           << "/prod/scripts/app/clinicals/p4p/..."
           << " //" << client << "/prod/scripts/app/clinicals/p4p/..."
           << "\n"
           << "\t-//depot/" << subBranch << branch << "/prod/sugarcrm/..."
           << " //" << client << "/prod/sugarcrm/..."
           << "\n"
           << "\t-//depot/" << subBranch << branch << "/techops/*/dailydml/..."
           << " //" << client << "/techops/*/dailydml/..."
           << "\n"
           << "\t-//depot/" << subBranch << branch << "/techops/*/dat/..."
           << " //" << client << "/techops/*/dat/..."
           << "\n"
           << "\t-//depot/" << subBranch << branch << "/techops/*/imports/..."
           << " //" << client << "/techops/*/onetimejobs/..."
           << "\n"
           << "\t-//depot/" << subBranch << branch << "/techops/*/oneoffscripts/..."
           << " //" << client << "/techops/*/oneoffscripts/..."
           << "\n"
           << "\t-//depot/" << subBranch << branch << "/techops/*/onetimejobs/..."
           << " //" << client << "/techops/*/onetimejobs/..."
           << "\n"
           << "\t-//depot/" << subBranch << branch << "/techops/dat/..."
           << " //" << client << "/techops/dat/..."
           << "\n"
           << "\t-//depot/" << subBranch << branch << "/techops/mis/..."
           << " //" << client << "/techops/mis/..."
           << "\n"
           << "\t-//depot/" << subBranch << branch << "/techops/sugar/..."
           << " //" << client << "/techops/sugar/..."
           << "\n";

    fmt::print("{}", writer.str());

    return EXIT_SUCCESS;
}
