#include "utils/RocksDB.hpp"
#include <string>
#include <memory>
#include "fmt/format.h"

void test_rocksdb(const std::string &database) {
  std::unique_ptr<rocksdb::DB> db(utils::open(database));
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

int main () {
  test_rocksdb("test_data");
}
