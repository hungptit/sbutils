// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <array>
#include <fstream>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"

#include "fmt/format.h"

#include "sbutils/FileSearch.hpp"
#include "sbutils/Print.hpp"
#include "sbutils/Timer.hpp"
#include "sbutils/Utils.hpp"

int main(int argc, char *argv[]) {
    using namespace boost;
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");

    // clang-format off
    desc.add_options()
        ("help,h", "Print this help")
        ("verbose,v", "Display searched data.")
        ("toJSON,j", po::value<std::string>(), "Output results in a JSON file.")
        ("folders,f", po::value<std::vector<std::string>>(), "Search folders.")
        ("file-stems,s", po::value<std::vector<std::string>>(), "File stems.")
        ("extensions,e", po::value<std::vector<std::string>>(), "File extensions.")
        ("pattern,p", po::value<std::string>(), "A search pattern.");
    // clang-format on

    po::positional_options_description p;
    p.add("folders", -1);
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc;
        return EXIT_SUCCESS;
    }

    auto verbose = vm.count("verbose");
    sbutils::ElapsedTime<sbutils::MILLISECOND> timer("Total time: ", verbose);
    
    std::string jsonFile;
    if (vm.count("toJSON")) {
        jsonFile = vm["toJSON"].as<std::string>();
    }

    using boost::filesystem::path;

    boost::system::error_code errcode;
    std::vector<std::string> folders;
    if (vm.count("folders")) {
        for (auto item : vm["folders"].as<std::vector<std::string>>()) {
            folders.emplace_back(path(item).string());
        }
    }

    std::vector<std::string> stems;
    if (vm.count("file-stems")) {
        stems = vm["file-stems"].as<std::vector<std::string>>();
    }

    std::vector<std::string> extensions;
    if (vm.count("extensions")) {
        extensions = vm["extensions"].as<std::vector<std::string>>();
    }

    std::string pattern;
    if (vm.count("pattern")) {
        pattern = vm["pattern"].as<std::string>();
    }

    // Search for files in the given folders.
    using path = boost::filesystem::path;
    using Container = std::vector<path>;
    sbutils::filesystem::SimpleVisitor<Container, sbutils::filesystem::NormalPolicy> visitor;
    Container searchFolders;
    for (auto item : folders) {
        searchFolders.emplace_back(path(item));
    }
    sbutils::filesystem::dfs_file_search(searchFolders, visitor);
    auto const & results = visitor.getResults();
    const sbutils::ExtFilter<std::vector<std::string>> f1(extensions);
    const sbutils::StemFilter<std::vector<std::string>> f2(stems);
    const sbutils::SimpleFilter f3(pattern);
    auto data =
        (pattern.empty()) ? sbutils::filter(results, f1, f2) : sbutils::filter(results, f1, f2, f3);

    if (verbose) {
        fmt::print("Search folders:\n");
        for (const auto &val : folders) {
            fmt::print("{}\n", val);
        }
        fmt::print("Number of files: {}\n", data.size());
        std::for_each(data.begin(), data.end(), [](auto const &val) {
            fmt::print("({0}, {1}, {2}, {3})\n", val.Path, val.Size, val.Permissions,
                       val.TimeStamp);
        });
    } else {
        fmt::print("Number of files: {}\n", data.size());
        std::for_each(data.begin(), data.end(),
                      [](auto const &val) { fmt::print("{0}\n", val.Path); });
    }

	// Write results to a JSON file.
    if (!jsonFile.empty()) {
        std::ostringstream os;
        {
            cereal::JSONOutputArchive oar(os);
            oar("Search results", results);
        }

        // Write to a JSON file
        std::ofstream myfile(jsonFile);
        myfile << os.str() << std::endl;
    }

    return 0;
}
