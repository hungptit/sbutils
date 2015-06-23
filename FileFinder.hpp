#ifndef FILEFINDER_HPP_
#define FILEFINDER_HPP_

#include <vector>
#include <string>
#include <tuple>
#include <boost/filesystem.hpp>
#include "boost/lexical_cast.hpp"
#include "boost/regex.hpp"

namespace Tools {
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

    // Use mixin design pattern to implement the file search algorithm.
    class FindFiles {
      public:
        virtual bool update(boost::filesystem::recursive_directory_iterator &dirIter) = 0;

        size_t search(const boost::filesystem::path &aPath) {
            boost::filesystem::recursive_directory_iterator endIter;
            boost::filesystem::recursive_directory_iterator dirIter(aPath);
            auto counter = 0;
            for (; dirIter != endIter; ++dirIter) {
                counter += update(dirIter);
            }
            return counter;
        }

      protected:
    };

    template <typename T> class FindAllFiles : public T {
      public:
        typedef std::tuple<std::string, boost::filesystem::perms, std::time_t> value_type;
        bool update(boost::filesystem::recursive_directory_iterator &dirIter) {
            const boost::filesystem::file_status fs = dirIter->status();
            if (fs.type() == boost::filesystem::regular_file) {
                auto fperm = fs.permissions();
                auto const currentFile = dirIter->path();
                const std::time_t t = boost::filesystem::last_write_time(currentFile);
                Data.emplace_back(std::make_tuple(currentFile.string(), fperm, t));
                return true;
            }
            return false;
        }

        std::vector<value_type> &getData() { return Data; }

      private:
        std::vector<value_type> Data;
    };
}
#endif
