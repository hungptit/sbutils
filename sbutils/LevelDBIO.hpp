#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "leveldb/cache.h"
#include "leveldb/db.h"

#include "DataStructures.hpp"
#include "Resources.hpp"
#include "Timer.hpp"
#include "Utils.hpp"

namespace utils {
    class Writer {
      public:
        Writer(const Writer &) = delete;
        Writer &operator=(const Writer &) = delete;

        explicit Writer(const std::string &dataFile) : DataFile(dataFile) {
            leveldb::Options options;
            // options.cache = leveldb::NewLRUCache(100 * 1048576);  // 100MB cache
            options.create_if_missing = true;
            leveldb::Status status = leveldb::DB::Open(options, DataFile, &Database);

            // Check that the database is opened successfully.
            if (false == status.ok()) {
                std::cerr << "Unable to open/create database \"" << DataFile << "\""
                          << std::endl;
                std::cerr << status.ToString() << std::endl;
            }
        }

        ~Writer() { delete Database; }

        void write(const std::string &key, const std::string &value) {
            leveldb::WriteOptions writeOptions;
            Database->Put(writeOptions, key, value);
        }

      private:
        leveldb::DB *Database;
        std::string DataFile;
    };

    class Reader {
      public:
        Reader(const Reader &) = delete;
        Reader &operator=(const Reader &) = delete;

        explicit Reader(const std::string &dataFile) : DataFile(dataFile) {
            leveldb::Options options;
            options.create_if_missing = false;
            leveldb::Status status = leveldb::DB::Open(options, DataFile, &Database);
            if (false == status.ok()) {
                std::cerr << "Unable to open database: \"" << DataFile << "\"" << std::endl;
                std::cerr << status.ToString() << std::endl;
            }
        }

        leveldb::DB *getDB() const { return Database; }

        std::vector<std::string> keys() {
            std::vector<std::string> allKeys;

            leveldb::Iterator *it = Database->NewIterator(leveldb::ReadOptions());
            for (it->SeekToFirst(); it->Valid(); it->Next()) {
                allKeys.emplace_back(it->key().ToString());
            }

            if (false == it->status().ok()) {
                std::cerr << "An error was found during the scan" << std::endl;
                std::cerr << it->status().ToString() << std::endl;
            }

            delete it;
            return allKeys;
        }

        std::string read(const std::string &aKey) {
            std::string results;
            leveldb::Iterator *it = Database->NewIterator(leveldb::ReadOptions());
            for (it->SeekToFirst(); it->Valid(); it->Next()) {
                if (it->key().ToString() == aKey) {
                    results = it->value().ToString();
                    break;
                }
            }

            if (false == it->status().ok()) {
                std::cerr << "An error was found during the scan" << std::endl;
                std::cerr << it->status().ToString() << std::endl;
            }

            delete it;
            return results;
        }

        ~Reader() { delete Database; }

      private:
        std::string DataFile;
        leveldb::DB *Database;
    };
}
