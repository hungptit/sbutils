#ifndef FILEFINDER_HPP_
#define FILEFINDER_HPP_

#include <vector>
#include <string>
#include <tuple>
#include <boost/filesystem.hpp>
#include "boost/lexical_cast.hpp"
#include "boost/regex.hpp"
#include "boost/program_options.hpp"
#include <boost/unordered_map.hpp>

namespace Tools {
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
