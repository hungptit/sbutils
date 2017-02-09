#include "boost/program_options.hpp"
#include <array>
#include <fstream>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

#include "sbutils/RocksDB.hpp"

#include <memory>

#include "sbutils/FileSearch.hpp"
#include "sbutils/FileUtils.hpp"
#include "sbutils/Resources.hpp"
#include "sbutils/Timer.hpp"

int main(int argc, char *argv[]) {
    using namespace boost;
    using path = boost::filesystem::path;
    using index_type = int;
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    std::string database;
    std::string cfgFile;

    // clang-format off
    desc.add_options()
        ("help,h", "Print this help")
        ("verbose,v", "Display verbose information.")
        ("folders,f", po::value<std::vector<std::string>>(), "Search folders.")
        ("config,c", po::value<std::string>(&cfgFile)->default_value(".mupdatedb.cfg"), "Search configuratiion.")
        ("database,d", po::value<std::string>(&database)->default_value(".database"), "File database.");
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
        return 0;
    }

    bool verbose = vm.count("verbose");
    utils::ElapsedTime<utils::MILLISECOND> totalTimer("Total time: ", verbose);
    
    std::vector<path> folders;

    if (vm.count("folders")) {
        auto list = vm["folders"].as<std::vector<std::string>>();
        std::for_each(list.begin(), list.end(), [&folders](auto const &item) {
            folders.emplace_back(utils::normalize_path(item));
        });
    } else {
        folders.emplace_back(boost::filesystem::current_path());
    }

    // Display input parameters if verbose is true
    if (verbose) {
        fmt::print("verbose: {}\n", verbose);
        fmt::print("config: {}\n", cfgFile);
        fmt::print("database: {}\n", database);
        auto printObj = [](auto const &item) {
            fmt::print("{}\n", item.string());
        };
        fmt::print("Search folders: \n");
        std::for_each(folders.cbegin(), folders.cend(), printObj);
    }

    // Build file information database
    using FileVisitor =
        utils::filesystem::Visitor<decltype(folders),
                                   utils::filesystem::NormalPolicy>;
    FileVisitor visitor;
    {
        utils::ElapsedTime<utils::MILLISECOND> searchTimer("Search time: ", verbose);
        utils::filesystem::dfs_file_search(folders, visitor);
    }
    
    // Save data to a rocksdb database.
    {
        utils::ElapsedTime<utils::SECOND> timer1("Serialization time: ", verbose);

        auto const results = visitor.getFolderHierarchy<index_type>();

        if (verbose) {
            utils::print<cereal::JSONOutputArchive>(results,
                                                    "Folder hierarchy");
        }

        results.info();
        utils::writeToRocksDB(database, results);
    }

    // Return
    return 0;
}
