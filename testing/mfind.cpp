#include <array>
#include <fstream>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"

#include "fmt/format.h"
#include "utils/FileSearch.hpp"
#include "utils/Print.hpp"
#include "utils/Serialization.hpp"
#include "utils/Timer.hpp"

int main(int argc, char *argv[]) {
    using namespace boost;
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    utils::Timer timer;

    // clang-format off
    desc.add_options()
        ("help,h", "Print this help")
        ("verbose,v", "Display searched data.")
        ("toJSON,j", po::value<std::string>(), "Output results in a JSON file.")
        ("folders,f", po::value<std::vector<std::string>>(), "Search folders.")
        ("file-stems,s", po::value<std::vector<std::string>>(), "File stems.")
        ("extensions,e", po::value<std::vector<std::string>>(), "File extensions.")
        ("search-strings,t", po::value<std::vector<std::string>>(), "File extensions.");
    // clang-format on

    po::positional_options_description p;
    p.add("folders", -1);
    po::variables_map vm;
    po::store(
        po::command_line_parser(argc, argv).options(desc).positional(p).run(),
        vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc;
        std::cout << "Examples:" << std::endl;
        std::cout << "\t mfind ./ -d .database" << std::endl;
        return EXIT_SUCCESS;
    }

    auto verbose = false;
    if (vm.count("verbose")) {
        verbose = true;
    }

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

    std::vector<std::string> searchStrings;
    if (vm.count("search-strings")) {
        searchStrings = vm["search-strings"].as<std::vector<std::string>>();
    }

    // Search for files in the given folders.
    using path = boost::filesystem::path;
    using Container = std::vector<path>;
    utils::filesystem::SimpleVisitor<Container, utils::filesystem::NormalPolicy>
        visitor;
    Container searchFolders;
    for (auto item : folders) {
        searchFolders.emplace_back(path(item));
    }
    utils::filesystem::dfs_file_search(searchFolders, visitor);
    auto results = visitor.getResults();
    utils::filesystem::ExtFilter<std::vector<std::string>> f1(extensions);
    utils::filesystem::StemFilter<std::vector<std::string>> f2(stems);
    auto data =
        utils::filesystem::filter(results.begin(), results.end(), f1, f2);

    if (verbose) {
        fmt::print("Search folders:\n");
        for (const auto &val : folders) {
            fmt::print("{}\n", val);
        }
        fmt::print("Number of files: {}\n", data.size());
        std::for_each(data.begin(), data.end(), [](auto const &val) {
            fmt::print("({0}, {1}, {2}, {3})\n", val.Path, val.Size, val.Permissions, val.TimeStamp);
        });
    } else {
        fmt::print("Number of files: {}\n", data.size());
        std::for_each(data.begin(), data.end(), [](auto const &val) {
            fmt::print("{0}\n", val.Path);
        });
    }

    if (!jsonFile.empty()) {
        std::ostringstream os;
        cereal::JSONOutputArchive output(os);
        utils::save(output, "Search results", results);
        std::ofstream myfile(jsonFile);
        myfile << os.str() << std::endl;
    }

    std::cout << "Total time: " << timer.toc() / timer.ticksPerSecond()
              << " seconds" << std::endl;
    return 0;
}
