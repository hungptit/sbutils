#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"
#include <array>
#include <fstream>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

#include "sbutils/Process.hpp"
#include "fmt/format.h"
#include "sbutils/FileUtils.hpp"

using path = boost::filesystem::path;

int main(int argc, char *argv[]) {
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");

    using path = boost::filesystem::path;
	std::vector<std::string> unitTests;
	
    // clang-format off
    desc.add_options()
        ("help,h", "Print this help")
        ("verbose,v", "Display verbose information.")
		("coverage", "Run code coverage.")
		("nodbh", "Do not use database")
		("tests,t", po::value<std::vector<std::string>>(&unitTests), "Test modules.");
    // clang-format on

    po::positional_options_description p;
    p.add("tests", -1);
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc;
        return EXIT_SUCCESS;
    }

	if (unitTests.empty()) {
		fmt::print("You need to provide a test module or test point!\n");
		return EXIT_SUCCESS;
	}
	
	std::vector<std::string> utestArgs;	

    bool verbose = vm.count("verbose");

	if (!vm.count("coverage")) {
		utestArgs.push_back("--nocoverage");
	}

	if (vm.count("nodbh")) {
		utestArgs.push_back("--nodbh");
	}

	std::for_each(unitTests.begin(), unitTests.end(), [&utestArgs](auto aTest){utestArgs.push_back(aTest);});
	
    fmt::MemoryWriter writer;

	// Execute given tests
	sbutils::CommandInfo args = std::make_tuple("utest", utestArgs);
	sbutils::CommandOutput outputs = sbutils::run(args, "./");

	writer << "Error code: " << std::get<2>(outputs) << "\n\n";
	writer << "Output: \n" << std::get<0>(outputs) << "\n";
	writer << "Error: \n" << std::get<1>(outputs) << "\n";
	
    fmt::print("{}", writer.str());
    return EXIT_SUCCESS;
}
