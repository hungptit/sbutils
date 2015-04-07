#ifndef FILEFINDER_HPP_
#define FILEFINDER_HPP_

#include <iostream>
#include <algorithm>
#include <iterator>
#include <utility>
#include <fstream>
#include <string>

#include <boost/filesystem.hpp>
#include "boost/lexical_cast.hpp"
#include "boost/regex.hpp"
#include "boost/program_options.hpp"
#include <boost/unordered_map.hpp>
// #include <tuple>

namespace Tools {
// TODO: Use tuple for value_type
template <typename FileSearchStrategy, typename FileExtSearchStrategy,
          typename FileNameSearchStrategy>
class FileFinder {
  public:
    typedef std::pair<std::string, boost::filesystem::perms> value_type;
    typedef std::vector<value_type> container_type;

    FileFinder() : Data() {}

    const std::vector<value_type> &getData() { return Data; };

    void print() {
        for (auto &item : Data) {
            std::cout << "(\"" << item.first << "\", " << item.second << ")"
                      << std::endl;
        }
        std::cout << "Number of files: " << Data.size() << std::endl;
    }

    void clear() { Data.clear(); }

    size_t search(const std::string &folderName,
                  typename FileExtSearchStrategy::extension_map_type &extmap,
                  const boost::regex &expressions) {
        boost::filesystem::path currentFolder(folderName);
        boost::filesystem::recursive_directory_iterator endIter;
        boost::filesystem::recursive_directory_iterator dirIter(currentFolder);
        for (; dirIter != endIter; ++dirIter) {
            boost::filesystem::file_status fs = dirIter->status();
            if (FileSearchStrategy::apply(fs)) {
                const std::string fileExtension =
                    dirIter->path().extension().string();
                if (FileExtSearchStrategy::apply(fileExtension, extmap)) {
                    const std::string fileName = dirIter->path().string();
                    if (FileNameSearchStrategy::apply(fileName, expressions)) {
                        Data.emplace_back(
                            std::make_pair(fileName, fs.permissions()));
                    }
                }
            }
        }
        return Data.size();
    }

  private:
    container_type Data;
};
}
#endif
