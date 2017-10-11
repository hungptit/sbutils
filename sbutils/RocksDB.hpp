#pragma once

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include "DataStructures.hpp"
#include "Resources.hpp"
#include "Utils.hpp"
#include "rocksdb/db.h"

namespace sbutils {
    template <typename T> rocksdb::DB *open(const std::string &database, T &&options) {
        rocksdb::DB *db = nullptr;
        rocksdb::Status status = rocksdb::DB::Open(std::forward<T>(options), database, &db);
        if (!status.ok()) {
            throw std::runtime_error(status.ToString());
        }
        return db;
    }

    rocksdb::DB *open(const std::string &database) {
        rocksdb::Options options;
        options.create_if_missing = true;
        return open(database, options);
    }

    // Write arguments to a key.
    template <typename OArchive, typename... Args>
    bool write(rocksdb::DB *db, const std::string &aKey, Args &&... args) {
        rocksdb::WriteOptions options;
        std::ostringstream os;
        {
            // Serialize the graph and vertex names into a string.
            OArchive oarchive(os);
            oarchive(std::forward<Args>(args)...);
        }
        rocksdb::Status dbstatus = db->Put(options, aKey, os.str());
        return dbstatus.ok();
    }

    // Serialize given arguments to a string then write serialized data to NoSQL database using a given key.
    template <typename OArchive, typename... Args>
    bool write(rocksdb::DB *db, const rocksdb::WriteOptions &options, const std::string &aKey,
               Args &&... args) {
        std::ostringstream os;
        {
            // Serialize the graph and vertex names into a string.
            OArchive oarchive(os);
            oarchive(std::forward<Args>(args)...);
        }
        rocksdb::Status dbstatus = db->Put(options, aKey, os.str());
        return dbstatus.ok();
    }

	// Read a value given a key then deserialize the string buffer to a given value.
	template <typename IArchive, typename T>
	T read(rocksdb::DB *db, const rocksdb::ReadOptions &options, const std::string &aKey) {
		T value;
		
		return value;
	}
	
    template <typename T> void writeToRocksDB(const std::string &database, const T &results) {
        std::unique_ptr<rocksdb::DB> db(sbutils::open(database));
        rocksdb::Status s;

        // Write all data using batch mode.
        rocksdb::WriteBatch batch;

        std::ostringstream os;

        // Write out all file information
        os.str(std::string());
        serialize<sbutils::DefaultOArchive>(results.AllFiles, os);
        batch.Put(sbutils::Resources::AllFileKey, os.str());

        // Write out graph information
        os.str(std::string());
        serialize<sbutils::DefaultOArchive>(results.Graph, os);
        batch.Put(sbutils::Resources::GraphKey, os.str());

        // Write out vertex information
        auto const &vertexes = results.Vertexes;
        os.str(std::string());
        serialize<sbutils::DefaultOArchive>(vertexes, os);
        batch.Put(sbutils::Resources::VertexKey, os.str());

        // Write out vids only
        std::vector<std::string> vids;
        vids.reserve(vertexes.size());
        std::for_each(vertexes.begin(), vertexes.end(),
                      [&vids](auto const &item) { vids.emplace_back(item.Path); });
        os.str(std::string());
        serialize<sbutils::DefaultOArchive>(vids, os);
        batch.Put(sbutils::Resources::VIDKey, os.str());

        // Write all vertexes into database and keys are the indexes.
        size_t counter = 0;
        auto writeObj = [&os, &batch, &counter](auto const &aVertex) {
            os.str(std::string());
            serialize<sbutils::DefaultOArchive>(aVertex, os);
            batch.Put(sbutils::to_fixed_string(9, counter), os.str());
            ++counter;
        };

        std::for_each(vertexes.begin(), vertexes.end(), writeObj);

        // Write all changes to the database
        auto const writeOpts = rocksdb::WriteOptions();
        s = db->Write(writeOpts, &batch);
        if (!s.ok()) {
            std::cerr << "Cannot write data to " << database << "\n";
        };
    }

    // TODO: A function which return all keys from a RocksDB database

    // TODO: A function which will serialize data to a string and store it in a given
    // key. Use param pack to reduce code duplication.

} // namespace sbutils
