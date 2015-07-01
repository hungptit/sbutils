#ifndef LevelDBIO_HPP_
#define LevelDBIO_HPP_

#include "leveldb/db.h"
#include "leveldb/cache.h"

namespace Tools {
    class Writer {
      public:
        Writer(const std::string &dataFile) : DataFile(dataFile) {
            leveldb::Options options;
            // options.cache = leveldb::NewLRUCache(100 * 1048576);  // 100MB cache
            options.create_if_missing = true;
            leveldb::Status status = leveldb::DB::Open(options, DataFile, &Database);

            // Check that the database is opened successfully.
            if (false == status.ok()) {
                std::cerr << "Unable to open/create database" << DataFile << std::endl;
                std::cerr << status.ToString() << std::endl;
            }
        }

        ~Writer() { delete Database; }

        void write(const std::string &key, const std::string &value) {
            leveldb::WriteOptions writeOptions;
            Database->Put(writeOptions, key, value);
        }

<<<<<<< HEAD
        // template <typename Container> void write(Container &data) {
        //     leveldb::WriteOptions writeOptions;
        //     for (const auto &val : data) {
        //         // TODO: Create a template function which can generate the JSON
        //         // string of a tuple.  Basically, we need file name, file
        //         // extension, permission, and time stamp. These parameters will
        //         // be used to query the information from the file database.
        //         const auto value = boost::lexical_cast<std::string>(std::get<2>(val)) +
        //             ":" +
        //             boost::lexical_cast<std::string>(std::get<3>(val));
        //         Database->Put(writeOptions, std::get<0>(val), value);
        //     }
        // }

=======
>>>>>>> acc5b9fc9c1451ccde039ef752809b51a12806a8
      private:
        leveldb::DB *Database;
        std::string DataFile;
    };

    class Reader {
      public:
        Reader(const std::string &dataFile) : DataFile(dataFile) {
            leveldb::Options options;
            options.create_if_missing = false;
            leveldb::Status status = leveldb::DB::Open(options, DataFile, &Database);
            if (false == status.ok()) {
                std::cerr << "Unable to open database" << DataFile << std::endl;
                std::cerr << status.ToString() << std::endl;
            }
        }

<<<<<<< HEAD
        std::vector<std::tuple<std::string, std::string>> read() {
            std::vector<std::tuple<std::string, std::string>> data;
=======
        std::vector<std::string> keys() {
            std::vector<std::string> allKeys;
>>>>>>> acc5b9fc9c1451ccde039ef752809b51a12806a8
            leveldb::Iterator *it = Database->NewIterator(leveldb::ReadOptions());
            for (it->SeekToFirst(); it->Valid(); it->Next()) {
<<<<<<< HEAD
                data.emplace_back(std::make_tuple(it->key().ToString(), it->value().ToString()));
=======
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
>>>>>>> acc5b9fc9c1451ccde039ef752809b51a12806a8
            }

            if (false == it->status().ok()) {
                std::cerr << "An error was found during the scan" << std::endl;
                std::cerr << it->status().ToString() << std::endl;
            }

            delete it;
<<<<<<< HEAD
            return data;
=======
            return results;
>>>>>>> acc5b9fc9c1451ccde039ef752809b51a12806a8
        }

        ~Reader() { delete Database; }

      private:
        std::string DataFile;
        leveldb::DB *Database;
    };
}
#endif
