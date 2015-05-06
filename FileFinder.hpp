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
        template <typename Container, typename Constraints>
        Container search(boost::filesystem::path &searchPath,
                         Constraints &constraints) {
            Container data;
            data.reserve(1000); // TODO: Do we need to do this?
            boost::filesystem::recursive_directory_iterator endIter;
            boost::filesystem::recursive_directory_iterator dirIter(searchPath);
            for (; dirIter != endIter; ++dirIter) {
                if (constraints.isValid(dirIter)) {
                    saveData(data, dirIter);
                }
            }
            return data;
        }

        template <typename Container, typename Writer>
        void write(const Container &data, const Writer &writer) {
            for (const auto &val : data) {
                writer.write(val);
            }
        }
    };

    class FileFinder {
      public:
        typedef std::pair<std::string, boost::filesystem::perms>
            value_type; // TODO: Use tuple for value_type

        FileFinder() : Data() {}

        std::vector<value_type> &getData() { return Data; };

        void print() {
            for (auto &item : Data) {
                std::cout << "(\"" << item.first << "\", " << item.second << ")"
                          << std::endl;
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
                Data.emplace_back(
                    std::make_pair(dirIter->path().string(), fs.permissions()));
            }
            return Data.size();
        }

        // Only find files or folder which satisfy given constraints.
        template <typename Permissions, typename Constraints>
        size_t search(const boost::filesystem::path &aPath,
                      const Constraints &cons) {
            boost::filesystem::recursive_directory_iterator endIter;
            boost::filesystem::recursive_directory_iterator dirIter(aPath);
            for (; dirIter != endIter; ++dirIter) {
                const boost::filesystem::file_status fs = dirIter->status();
                if (Permissions::isValid(fs)) {
                    const boost::filesystem::path p = dirIter->path();
                    if (cons.isValid(p)) {
                        Data.emplace_back(std::make_pair(
                            dirIter->path().string(), fs.permissions()));
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
