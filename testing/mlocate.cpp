#include "boost/program_options.hpp"
#include <array>
#include <fstream>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

#include "utils/FileSearch.hpp"
#include "utils/FileUtils.hpp"
#include "utils/FolderDiff.hpp"
#include "utils/LevelDBIO.hpp"
#include "utils/Resources.hpp"
#include "utils/Timer.hpp"

template <typename Container>
void print(const Container &results) {
    fmt::MemoryWriter writer;
    writer << "Search results: \n";
    std::for_each(results.begin(), results.end(),
                  [&writer](auto const &item) { writer << item.Path << "\n"; });
    ;
    fmt::print("{}", writer.str());
}

int main(int argc, char *argv[]) {
    namespace po = boost::program_options;
    using path = boost::filesystem::path;
    using IArchive = utils::DefaultIArchive;
    using Index = int;

    utils::ElapsedTime<utils::MILLISECOND> timer("Total time: ");

    po::options_description desc("Allowed options");

    // clang-format off
    desc.add_options()
        ("help,h", "Print this help")
        ("verbose,v", "Display searched data.")
        ("keys,k", "List all keys.")
        ("folders,f", po::value<std::vector<std::string>>(), "Search folders.")
        ("stems,s", po::value<std::vector<std::string>>(), "File stems.")
        ("extensions,e", po::value<std::vector<std::string>>(), "File extensions.")
        ("pattern,p", po::value<std::string>(), "Search string pattern.")
        ("database,d", po::value<std::string>(), "File database.");
    // clang-format on

    po::positional_options_description p;
    p.add("pattern", -1);
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << "Usage: viewer [options]\n";
        std::cout << desc;
        std::cout << "Examples:" << std::endl;
        std::cout << "\t viewer -d .database -s AutoFix" << std::endl;
        return 0;
    }

    bool verbose = false;
    if (vm.count("verbose")) {
        verbose = true;
    }

    // Search folders
    std::vector<std::string> folders;
    if (vm.count("folders")) {
        folders = vm["folders"].as<std::vector<std::string>>();
        std::sort(folders.begin(), folders.end());
    }

    // Get file stems
    std::vector<std::string> stems;
    if (vm.count("stems")) {
        stems = vm["stems"].as<std::vector<std::string>>();
    }

    // Get file extensions
    std::vector<std::string> extensions;
    if (vm.count("extensions")) {
        extensions = vm["extensions"].as<std::vector<std::string>>();
    }

    // Get file extensions
    std::string pattern;
    if (vm.count("pattern")) {
        pattern = vm["pattern"].as<std::string>();
    }

    // Get file database
    std::string database;
    if (vm.count("database")) {
        database = vm["database"].as<std::string>();
    } else {
        database = (path(utils::Resources::Database)).string();
    }

    if (verbose) {
        std::cout << "Database: " << database << std::endl;
    }

    {
        using Container = std::vector<utils::FileInfo>;
        Container allFiles = utils::read_baseline<Container>(database, folders, verbose);
        Container results;
        if (pattern.empty()) {
            utils::StemFilter<decltype(stems)> stemFilter(stems);
            utils::ExtFilter<decltype(extensions)> extFilter(extensions);
            results = utils::filter(allFiles.cbegin(), allFiles.cend(), stemFilter, extFilter);
        } else {
            utils::StemFilter<decltype(stems)> stemFilter(stems);
            utils::ExtFilter<decltype(extensions)> extFilter(extensions);
            utils::SimpleFilter patternFilter(pattern);
            print(utils::filter(allFiles.begin(), allFiles.end(), extFilter, patternFilter, stemFilter, extFilter));
            print(utils::filter_nopack(allFiles.begin(), allFiles.end(), patternFilter, stemFilter, extFilter));
        }
        print(results);
    }
}
