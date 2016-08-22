#ifndef RocksDB_hpp
#define RocksDB_hpp

#include <memory>
#include <string>
#include <utility>

#include "rocksdb/db.h"

#include "DataStructures.hpp"
#include "Resources.hpp"
#include "Utils.hpp"

namespace utils {
    template <typename T>
    rocksdb::DB *open(const std::string &database, T &&options) {
        rocksdb::DB *db = nullptr;
        rocksdb::Status status =
            rocksdb::DB::Open(std::forward<T>(options), database, &db);

        assert(status.ok());
        assert(db);

        return db;
    }

    rocksdb::DB *open(const std::string &database) {
        rocksdb::Options options;
        options.create_if_missing = true;
        // options.compression = rocksdb::kBZip2Compression;
        // options.compression = rocksdb::kLZ4HCCompression;
        // options.compression = rocksdb::kSnappyCompression;
        // options.compression = rocksdb::kZlibCompression;
        return open(database, options);
    }

    template <typename T>
    void writeToRocksDB(const std::string &database, const T &results) {
        std::unique_ptr<rocksdb::DB> db(utils::open(database));
        rocksdb::Status s;

        // Write all data using batch mode.
        rocksdb::WriteBatch batch;

        std::ostringstream os;

        // Write out all file information
        os.str(std::string());
        serialize<utils::DefaultOArchive>(results.AllFiles, os);
        batch.Put(utils::Resources::AllFileKey, os.str());

        // Write out graph information
        os.str(std::string());
        serialize<utils::DefaultOArchive>(results.Graph, os);
        batch.Put(utils::Resources::GraphKey, os.str());

        // Write out vertex information
        auto const &vertexes = results.Vertexes;
        os.str(std::string());
        serialize<utils::DefaultOArchive>(vertexes, os);
        batch.Put(utils::Resources::VertexKey, os.str());

        // Write out vids only
        std::vector<std::string> vids;
        vids.reserve(vertexes.size());
        std::for_each(
            vertexes.begin(), vertexes.end(),
            [&vids](auto const &item) { vids.emplace_back(item.Path); });
        os.str(std::string());
        serialize<utils::DefaultOArchive>(vids, os);
        batch.Put(utils::Resources::VIDKey, os.str());

        // Write all vertexes into database and keys are the indexes.
        size_t counter = 0;
        auto writeObj = [&os, &batch, &counter](auto const &aVertex) {
          os.str(std::string());
          serialize<utils::DefaultOArchive>(aVertex, os);
          batch.Put(utils::to_fixed_string(9, counter), os.str());
          ++counter;
        };

        std::for_each(vertexes.begin(), vertexes.end(), writeObj);
        
        // Write all changes to the database
        auto const writeOpts = rocksdb::WriteOptions();
        s = db->Write(writeOpts, &batch);
        assert(s.ok()); // Write all changes to the database
    }
}

#endif
