// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
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
#include "sbutils/Process.hpp"
#include "sbutils/Timer.hpp"

using path = boost::filesystem::path;

int main(int argc, char *argv[]) {
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");

    using path = boost::filesystem::path;
    std::vector<std::string> unitTests;

    bool nodbh = true;
    bool failedOnly = true;
    bool nocoverage = true;

    // clang-format off
    desc.add_options()
        ("help,h", "Print this help")
        ("verbose,v", "Display verbose information.")
		("nocoverage", po::value<bool>(&nocoverage)->default_value(true), "Run code coverage.")
		("nodbh", po::value<bool>(&nodbh)->default_value(true), "Do not use database")
		("failedonly", po::value<bool>(&failedOnly)->default_value(true), "only dislay failed tests")
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

    if (nocoverage) {
        utestArgs.push_back("--nocoverage");
    }

    if (nodbh) {
        utestArgs.push_back("--nodbh");
    }

    if (failedOnly) {
        utestArgs.push_back("--failedonly");
    }

    fmt::MemoryWriter writer;
    std::vector<std::tuple<std::string, sbutils::CommandOutput>> results;
    auto testObj = [&results, &utestArgs, &writer](std::string &aTest) {
        sbutils::ElapsedTime<sbutils::SECOND> timer(aTest + ": ");
        auto newArgs(utestArgs);
        newArgs.push_back(aTest);
        sbutils::CommandInfo args = std::make_tuple("utest", newArgs);
        sbutils::CommandOutput outputs = sbutils::run(args, "./");
        writer << "Output: \n" << std::get<0>(outputs) << "\n";
        results.push_back(std::make_tuple(aTest, outputs));

    };

    std::for_each(unitTests.begin(), unitTests.end(), testObj);

    std::for_each(results.begin(), results.end(),
                  [&writer](auto item) { writer << std::get<0>(item); });

    fmt::print("{}", writer.str());
    return EXIT_SUCCESS;
}
