#include "boost/program_options.hpp"
#include <array>
#include <fstream>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

<<<<<<< HEAD
#include "utils/FileSearch.hpp"
=======
#include "utils/DatabaseUtils.hpp"
#include "utils/FileSearch.hpp"
#include "utils/FileUtils.hpp"
>>>>>>> baa8b01af6793909ae056f40248903e09cb097e3
#include "utils/LevelDBIO.hpp"
#include "utils/Resources.hpp"
#include "utils/Serialization.hpp"
#include "utils/Timer.hpp"
#include "utils/FolderDiff.hpp"

int main(int argc, char *argv[]) {
    namespace po = boost::program_options;
    using path = boost::filesystem::path;
    using IArchive = utils::DefaultIArchive;
<<<<<<< HEAD
    using Index = int;
=======
>>>>>>> baa8b01af6793909ae056f40248903e09cb097e3

    po::options_description desc("Allowed options");

    // clang-format off
  desc.add_options()
    ("help,h", "Print this help")
    ("verbose,v", "Display searched data.")
<<<<<<< HEAD
    ("info,i", "Display general information about stored data.")
    ("parallel,p", "Use threaded version of viewer.")
=======
>>>>>>> baa8b01af6793909ae056f40248903e09cb097e3
    ("keys,k", "List all keys.")
    ("folders,f", po::value<std::vector<std::string>>(), "Search folders.")
    ("stems,s", po::value<std::vector<std::string>>(), "File stems.")
    ("extensions,e", po::value<std::vector<std::string>>(), "File extensions.")
    ("strings,t", po::value<std::vector<std::string>>(), "Search string")
    ("database,d", po::value<std::string>(), "File database.");
    // clang-format on

    po::positional_options_description p;
    p.add("folders", -1);
    po::variables_map vm;
    po::store(
        po::command_line_parser(argc, argv).options(desc).positional(p).run(),
        vm);
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

<<<<<<< HEAD
    bool info = false;
    if (vm.count("info")) {
        info = true;
    }

    bool isThreaded = false;
    if (vm.count("parallel")) {
        isThreaded = true;
    }

=======
>>>>>>> baa8b01af6793909ae056f40248903e09cb097e3
    // Search folders
    std::vector<std::string> folders;
    if (vm.count("folders")) {
        folders = vm["folders"].as<std::vector<std::string>>();
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
    std::vector<std::string> searchStrings;
    if (vm.count("strings")) {
        searchStrings = vm["strings"].as<std::vector<std::string>>();
    }

    // Get file database
    std::string dataFile;
    if (vm.count("database")) {
        dataFile = vm["database"].as<std::string>();
    } else {
        dataFile = (path(utils::Resources::Database)).string();
    }

    if (verbose) {
        std::cout << "Database: " << dataFile << std::endl;
    }

    utils::ElapsedTime<utils::MILLISECOND> timer;
    utils::Reader reader(dataFile);
    if (vm.count("keys")) {
        for (auto &aKey : reader.keys()) {
            std::cout << aKey << std::endl;
        }
    } else {
<<<<<<< HEAD
        // Now find all files which satisfy given constraints.
        std::vector<utils::FileInfo> allFiles;
        if (folders.empty()) {
            utils::ElapsedTime<utils::MILLISECOND> t("Deserialization time: ");
            std::istringstream is(reader.read(utils::Resources::AllFileKey));
            IArchive input(is);
            input(allFiles);
            if (verbose) {
                fmt::print("Number of files: {0}\n", allFiles.size());
            }
        } else {
            // Only read the file information for given folders.
            std::istringstream is(reader.read(utils::Resources::GraphKey));
            std::vector<std::string> vids;
            std::vector<Index> v;
            std::vector<Index> e;
            IArchive input(is);
            input(vids, v, e);
            utils::SparseGraph<int> g(v, e, true);

            if (verbose) {
                fmt::print("Number of vertexes: {0}\n", vids.size());
                fmt::print("Number of edges: {0}\n", e.size());
            }

            // Display detail information about file hierarchy tree.
            if (info) {
                utils::graph_info(g);
                fmt::MemoryWriter writer;
                size_t counter = 0;
                for (auto item : vids) {
                    writer << "vid[" << counter << "] = " << item << "\n";
                    counter++;
                }
                std::cout << writer.str();
            }

            // Now find indexes for given folders
            std::vector<Index> indexes;
            for (auto item : folders) {
                auto aKey = item;
                if (item.back() == '/') {
                  aKey = item.substr(0, item.size() - 1);
                  std::cout << aKey << "-----\n";
                }

                auto it = std::lower_bound(vids.begin(), vids.end(), aKey);
                if (*it == item) {
                    auto vid = std::distance(vids.begin(), it);
                    indexes.push_back(vid);
                    fmt::print("vid[\"{0}\"] = {1}\n", aKey, vid);
                } else {
                    fmt::print(
                        "Could not find database for file in {} directory\n",
                        item);
                }
            }
        }

        {
            utils::ExtFilter<decltype(extensions)> f1(extensions);
            utils::StemFilter<decltype(stems)> f2(stems);
            auto results =
                utils::filter(allFiles.begin(), allFiles.end(), f1, f2);

            // Display the search results
            fmt::MemoryWriter writer;
            for (auto item : results) {
                writer << std::get<0>(item) << "\n";
            }
            fmt::print("{}", writer.str());
        }
=======
        using Container = std::vector<utils::FileInfo>;
        auto allFiles = utils::read_baseline<Container>(reader, folders, verbose);
        auto results = utils::filter(allFiles, extensions, stems);
        fmt::MemoryWriter writer;
        writer << "Search results: \n";
        for (auto item : results) {
            writer << std::get< utils::filesystem::PATH>(item) << "\n";
        }
        fmt::print("{}", writer.str());
>>>>>>> baa8b01af6793909ae056f40248903e09cb097e3
    }
}
