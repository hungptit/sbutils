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

namespace Tools {
    /**
     * This class will search for files/folders and write to the output stream
     * which can be files, databases, or stdout.
     */
    class Finder {
      public:
        typedef std::tuple<std::string, boost::filesystem::perms, std::time_t> value_type;

        /**
         * Build the file database of the current folder.
         *
         * @param searchPath
         */
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

        // TODO: Move this method out of this class
        bool write(const std::string &database) {
            leveldb::Options options;
            // options.cache = leveldb::NewLRUCache(100 * 1048576);  // 100MB cache
            options.create_if_missing = true;

            // Open the database
            leveldb::DB *db;
            leveldb::Status status = leveldb::DB::Open(options, database, &db);

            if (!status.ok())
                std::cerr << status.ToString() << std::endl;

            if (false == status.ok()) {
                std::cerr << "Unable to open/create database" << database << std::endl;
                std::cerr << status.ToString() << std::endl;
                return false;
            }

            leveldb::WriteOptions writeOptions;
            for (const auto &val : Data) {
                db->Put(writeOptions, std::get<0>(val), boost::lexical_cast<std::string>(std::get<1>(val)));
            }

            // Close the database
            delete db;
            return true;
        }

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
