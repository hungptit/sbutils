#ifndef RocksDB_hpp
#define RocksDB_hpp

#include <string>
#include <memory>
#include <utility>

#include "rocksdb/db.h"

namespace utils {
    template <typename T>
    rocksdb::DB *open(const std::string &database, T &&options) {
        rocksdb::DB *db = nullptr;
        rocksdb::Status status = rocksdb::DB::Open(std::forward<T>(options), database, &db);

        assert(status.ok());
        assert(db);

        return db;
    }

    rocksdb::DB *open(const std::string &database) {
        rocksdb::Options options;
        options.create_if_missing = true;
        return open(database, options);
    }
}

#endif
