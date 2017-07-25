// This is an independent project of an individual developer. Dear PVS-Studio,
// please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "boost/program_options.hpp"
#include <array>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "sbutils/FileSearch.hpp"
#include "sbutils/FileUtils.hpp"
#include "sbutils/Resources.hpp"
#include "sbutils/RocksDB.hpp"
#include "sbutils/Timer.hpp"

int main(int argc, char *argv[]) {
    using namespace boost;
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    std::string database;
    std::vector<std::string> keys;

    // clang-format off
    desc.add_options()
        ("help,h", "Print this help")
        ("verbose,v", "Display verbose information.")
        ("all-keys,a", "Display all keys in a given database")
        ("keys,k", po::value<std::vector<std::string>>(&keys), "List the content of given keys.")
        ("database,d", po::value<std::string>(&database)->default_value(sbutils::Resources::Database), "File database.");
    // clang-format on

    po::positional_options_description p;
    p.add("database", -1);
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc;
        return 0;
    }

    bool verbose = vm.count("verbose");
    bool displayAllKeys = vm.count("all-keys");

    // Display input parameters if verbose is true
    if (verbose) {
        fmt::print("verbose: {}\n", verbose);
        fmt::print("all-keys: {}\n", displayAllKeys);
        fmt::print("database: {}\n", database);
    }

    sbutils::ElapsedTime<sbutils::MILLISECOND> timer("Total time: ", verbose);

    // Open a given database
    rocksdb::Options options;
    options.create_if_missing = false;
    std::unique_ptr<rocksdb::DB> db(sbutils::open(database, options));
    assert(db != nullptr);

    if (displayAllKeys) {
        keys.clear();
        std::unique_ptr<rocksdb::Iterator> it(db->NewIterator(rocksdb::ReadOptions()));
        for (it->SeekToFirst(); it->Valid(); it->Next()) {
            assert(it->status().ok());
            keys.emplace_back(it->key().ToString());
        }

        fmt::print("Number of keys: {}\n", keys.size());
        fmt::MemoryWriter writer;
        std::for_each(keys.begin(), keys.end(),
                      [&writer](const auto &aKey) { writer << aKey << "\n"; });
        fmt::print("{0}\n", writer.str());
    } else if (!keys.empty()) {
        // Display the key-value of a given list of keys
        std::for_each(keys.begin(), keys.end(), [&db](auto const &aKey) {
            std::string value;
            rocksdb::Status s = db->Get(rocksdb::ReadOptions(), aKey, &value);
            assert(s.ok());
            fmt::print("{0} : {1}\n", aKey, value);
        });
    } else {
        // Display a summary of a given RocksDB database.
        std::size_t counter = 0;
        std::size_t valueSizes = 0;
        std::unique_ptr<rocksdb::Iterator> it(db->NewIterator(rocksdb::ReadOptions()));
        for (it->SeekToFirst(); it->Valid(); it->Next()) {
            ++counter;
            const rocksdb::Slice aSlice = it->value();
            valueSizes += aSlice.size();
        }
        fmt::print("Number of keys: {}\n", counter);
        fmt::print("Sizeof all values (bytes): {}\n", valueSizes);
    }

    return 0;
}
