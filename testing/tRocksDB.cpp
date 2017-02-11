#include "fmt/format.h"

#include "sbutils/RocksDB.hpp"

#include <memory>
#include <string>

// void test_rocksdb(const std::string &database) {
//   std::unique_ptr<rocksdb::DB> db(utils::open(database));
//   std::string key("aKey");
//   std::string value("This is a value");
//   std::string val;
//   db->Put(rocksdb::WriteOptions(), key, value);
//   db->Get(rocksdb::ReadOptions(), key, &val);
//   db->Delete(rocksdb::WriteOptions(), key);

//   auto s = db->Get(rocksdb::ReadOptions(), key, &val);
//   fmt::print("s = {}\n", s.ok());

//   fmt::print("{0} -> {1}\n{0} -> {2}\n", key, value, val);
// }

void test_rocksdb(const std::string &database) {
    std::unique_ptr<rocksdb::DB> db(utils::open(database));

    {
        std::string key("aKey");
        std::string value("This is a value");
        std::string val;
        db->Put(rocksdb::WriteOptions(), key, value);
        db->Get(rocksdb::ReadOptions(), key, &val);
        db->Delete(rocksdb::WriteOptions(), key);

        auto s = db->Get(rocksdb::ReadOptions(), key, &val);
        fmt::print("s = {}\n", s.ok());

        fmt::print("{0} -> {1}\n{0} -> {2}\n", key, value, val);
    }

    {
        std::string key("_graph_");
        std::string val;
        db->Get(rocksdb::ReadOptions(), key, &val);
        auto s = db->Get(rocksdb::ReadOptions(), key, &val);
        fmt::print("s = {}\n", s.ok());
        fmt::print("{0} -> {1}\n", key, val);
    }
}

int main() { test_rocksdb(".database"); }
