#ifndef FILEFINDER_HPP_
#define FILEFINDER_HPP_

#include <vector>
#include <string>
#include <tuple>
#include <boost/filesystem.hpp>
#include "boost/lexical_cast.hpp"
#include "boost/regex.hpp"

#include "leveldb/db.h"
#include "leveldb/cache.h"

#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "Json.hpp"

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

        template <typename Container> void write(Container &data) {
            leveldb::WriteOptions writeOptions;
            for (const auto &val : data) {
                // TODO: Create a template function which can generate the JSON
                // string of a tuple.  Basically, we need file name, file
                // extension, permission, and time stamp. These parameters will
                // be used to query the information from the file database.
                const auto value = boost::lexical_cast<std::string>(std::get<2>(val)) + ":" + boost::lexical_cast<std::string>(std::get<2>(val));
                Database->Put(writeOptions, std::get<0>(val), value);
            }
        }
                
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

        void read() {
            leveldb::Iterator *it = Database->NewIterator(leveldb::ReadOptions());

            for (it->SeekToFirst(); it->Valid(); it->Next()) {
                std::cout << it->key().ToString() << " : " << it->value().ToString() << std::endl;
            }

            if (false == it->status().ok()) {
                std::cerr << "An error was found during the scan" << std::endl;
                std::cerr << it->status().ToString() << std::endl;
            }

            delete it;
        }

        ~Reader() { delete Database; }

      private:
        std::string DataFile;
        leveldb::DB *Database;
    };

    /**
     * This class will search for files/folders and write to the output stream
     * which can be files, databases, or stdout.
     */
    class Finder {
      public:
        typedef std::tuple<std::string, boost::filesystem::perms, std::time_t> value_type;

        void search(const boost::filesystem::path &searchPath) {
            boost::filesystem::recursive_directory_iterator endIter;
            boost::filesystem::recursive_directory_iterator dirIter(searchPath);
            for (; dirIter != endIter; ++dirIter) {
                const boost::filesystem::file_status fs = dirIter->status();
                if (fs.type() == boost::filesystem::regular_file) {
                    auto fperm = fs.permissions();
                    auto const currentFile = dirIter->path();
                    const std::time_t t = boost::filesystem::last_write_time(currentFile);
                    Data.emplace_back(std::make_tuple(currentFile.string(), fperm, t));
                }
            }
        }

        std::vector<value_type> &getData() { return Data; }

        void reset() { Data.clear(); }

      private:
        std::vector<value_type> Data;
    };

    /**
     * This class will find all edited files using the below rules
     *
     * If the search extensions is specified then we only search for file with
     * valid extensions otherwise we search for all files.
     *
     * After we get a list of edited file we will compare them with the baseline
     * data i.e. timestamp and only keep files which have the same time-stamp as
     * that of the baseline.
     *
     * @todo: Speed up this class using threads?
     */

    class FindEditedFiles {
      public:
        typedef std::tuple<std::string, boost::filesystem::perms, std::time_t> value_type;

        void search(const boost::filesystem::path &searchPath) {
            boost::filesystem::recursive_directory_iterator endIter;
            boost::filesystem::recursive_directory_iterator dirIter(searchPath);
            for (; dirIter != endIter; ++dirIter) {
                const boost::filesystem::file_status fs = dirIter->status();
                if (fs.type() == boost::filesystem::regular_file) {
                    auto fperm = fs.permissions();
                    if (fperm & boost::filesystem::owner_write) {
                        auto const currentFile = dirIter->path();
                        const std::time_t t = boost::filesystem::last_write_time(currentFile);
                        Data.emplace_back(std::make_tuple(currentFile.string(), fperm, t));
                    }
                }
            }
        }

        template <typename Container> void search(const boost::filesystem::path &searchPath, Container &searchExtensions) {
            boost::filesystem::recursive_directory_iterator endIter;
            boost::filesystem::recursive_directory_iterator dirIter(searchPath);
            for (; dirIter != endIter; ++dirIter) {
                const boost::filesystem::file_status fs = dirIter->status();
                if (fs.type() == boost::filesystem::regular_file) {
                    auto fperm = fs.permissions();
                    if (fperm & boost::filesystem::owner_write) {
                        auto const currentFile = dirIter->path();
                        const std::string fileExtension = currentFile.extension().string();
                        if (std::find(searchExtensions.begin(), searchExtensions.end(), fileExtension) !=
                            searchExtensions.end()) {
                            const std::time_t t = boost::filesystem::last_write_time(currentFile);
                            Data.emplace_back(std::make_tuple(currentFile.string(), fperm, t));
                        }
                    }
                }
            }
        }

        // @todo Use Spirit and Qi to improve the performance.
        void print() {
            for (auto &item : Data) {
                std::cout << "(" << std::get<0>(item) << ", " << std::get<1>(item) << "," << std::get<2>(item) << ")"
                          << std::endl;
            }
            std::cout << "Number of files: " << Data.size() << std::endl;
        }

        std::vector<value_type> &getData() { return Data; }

      private:
        std::vector<value_type> Data;
    };

    class FileFinder {
      public:
        typedef std::tuple<std::string, boost::filesystem::perms> value_type; // TODO: Use tuple for value_type

        FileFinder() : Data() {}

        std::vector<value_type> &getData() { return Data; };

        void print() {
            for (auto &item : Data) {
                std::cout << "(\"" << std::get<0>(item) << "\", " << std::get<1>(item) << ")" << std::endl;
            }
            std::cout << "Number of files: " << Data.size() << std::endl;
        }

        void clear() { Data.clear(); }

        // Default search algorithm which find everything inside a given folder.
        size_t search(const boost::filesystem::path &aPath) {
            boost::filesystem::recursive_directory_iterator endIter;
            boost::filesystem::recursive_directory_iterator dirIter(aPath);
            for (; dirIter != endIter; ++dirIter) {
                const boost::filesystem::file_status fs = dirIter->status();
                Data.emplace_back(std::make_tuple(dirIter->path().string(), fs.permissions()));
            }
            return Data.size();
        }

        // Only find files or folder which satisfy given constraints.
        template <typename Permissions, typename Constraints>
        size_t search(const boost::filesystem::path &aPath, const Constraints &cons) {
            boost::filesystem::recursive_directory_iterator endIter;
            boost::filesystem::recursive_directory_iterator dirIter(aPath);
            for (; dirIter != endIter; ++dirIter) {
                const boost::filesystem::file_status fs = dirIter->status();
                if (Permissions::isValid(fs)) {
                    const boost::filesystem::path p = dirIter->path();
                    if (cons.isValid(p)) {
                        Data.emplace_back(std::make_tuple(dirIter->path().string(), fs.permissions()));
                    }
                }
            }
            return Data.size();
        }

      private:
        std::vector<value_type> Data;
    };
}
#endif
