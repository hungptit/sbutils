#include "boost/program_options.hpp"
#include <array>
#include <fstream>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

#include "utils/RocksDB.hpp"

#include <memory>

#include "utils/FileSearch.hpp"
#include "utils/FileUtils.hpp"
#include "utils/LevelDBIO.hpp"
#include "utils/Resources.hpp"
#include "utils/Timer.hpp"

int main(int argc, char *argv[]) {
    using namespace boost;
    using path = boost::filesystem::path;

    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    std::string dataFile;
    std::vector<std::string> keys;

    // clang-format off
    desc.add_options()
        ("help,h", "Print this help")
        ("verbose,v", "Display verbose information.")
        ("a,all-keys", "Display all keys in a given database")
        ("k,key", po::value<std::vector<std::string>>(&keys), "List the content of given keys.")
        ("database,d", po::value<std::string>(&database)->default_value(".database"), "File database.");
    // clang-format on

    bool verbose = vm.count("verbose");
    bool displayAllKeys = = vm.count("all-keys");

    po::positional_options_description p;
    p.add("database", -1);
    po::variables_map vm;
    po::store(
        po::command_line_parser(argc, argv).options(desc).positional(p).run(),
        vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc;
        return 0;
    }

    // Display input parameters if verbose is true
    if (verbose) {
        fmt::print("verbose: {}\n", verbose);
        fmt::print("all-keys: {}\n", displayAllKeys);
        fmt::print("database: {}\n", database);
    }

    std::unique_ptr<rocksdb::DB> db(utils::open(dataFile));

    if (displayAllKeys) {
        // Display all keys
    } else if (!keys.empty()) {
        // Display the key-value of a given list of keys
    } else {
        // Display a summary of a given RocksDB database.
    }
    
    return 0;
}
