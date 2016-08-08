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
        serialize<utils::DefaultOArchive>(results.AllFiles, utils::Resources::AllFileKey, os);
        batch.Put(utils::Resources::AllFileKey, os.str());
        
        // Write out graph information
        os.str(std::string());
        serialize<utils::DefaultOArchive>(results.Graph, utils::Resources::GraphKey, os);
        batch.Put(utils::Resources::GraphKey, os.str());

        // Write out vertex information
        os.str(std::string());
        serialize<utils::DefaultOArchive>(results.Vertexes, utils::Resources::VertexKey, os);
        batch.Put(utils::Resources::VertexKey, os.str());        

        // Write all changes to the database
        s = db->Write(rocksdb::WriteOptions(), &batch);
        assert(s.ok());

        // Write all vertexes into database and keys are the indexes.
        size_t counter = 0;
        auto const writeOpts = rocksdb::WriteOptions();
        for (auto const &aVertex : results.Vertexes) {
            os.str(std::string());
            std::string aKey = utils::to_fixed_string(9, counter);
            serialize<utils::DefaultOArchive>(aVertex, aKey, os);
            db->Put(writeOpts, aKey, os.str());
            ++counter;
        }
    }
}

#endif
