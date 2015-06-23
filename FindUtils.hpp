#ifndef FindUtils_hpp
#define FindUtils_hpp

#include <vector>
#include <string>
#include <tuple>

#include <boost/filesystem.hpp>
#include "boost/lexical_cast.hpp"
#include "boost/regex.hpp"

namespace Tools {
    // TODO: How to serialize/deserialize tuple
    enum DataField { FullPath, Extension, Permission, TimeStamp, Checksum };
    typedef std::tuple<std::string, std::string> BasicFileInfo;
    typedef std::tuple<std::string, std::string, boost::filesystem::perms, std::time_t> ExtendedFileInfo;
    typedef std::tuple<std::string, std::string, boost::filesystem::perms, std::time_t, std::array<int, 4>> CompleteFileInfo;

    // Implement Template Method design pattern using C++ template. This approach will minize the code duplication.
    template <typename T> class Finder {
      public:
        typedef T value_type;
        void search(const boost::filesystem::path &aPath) {
            boost::filesystem::recursive_directory_iterator endIter;
            boost::filesystem::recursive_directory_iterator dirIter(aPath);
            for (; dirIter != endIter; ++dirIter) {
                update(dirIter);
            }
        }

        const std::vector<value_type> &getData() const { return Data; }

      protected:
        virtual void update(boost::filesystem::recursive_directory_iterator &dirIter) = 0;
        std::vector<T> Data;
    };

    // Build a simple file database which containts file paths and extensions
    template <typename T> class BuildFileDatabase : public T {
      protected:
        void update(boost::filesystem::recursive_directory_iterator &dirIter) {

            const boost::filesystem::file_status fs = dirIter->status();
            if (fs.type() == boost::filesystem::regular_file) {
                auto const currentFile = dirIter->path();
                T::Data.emplace_back(std::make_tuple(currentFile.string(), currentFile.extension().string()));
            }
        }
    };

    // Build a edited file database which containts file paths, extensions, permissions, and last write time.
    template <typename T>
    class BuildEditedFileDataBase : public T {
      protected:
        void update(boost::filesystem::recursive_directory_iterator &dirIter) {
            const boost::filesystem::file_status fs = dirIter->status();
            if (fs.type() == boost::filesystem::regular_file) {
                auto const currentFile = dirIter->path();
                T::Data.emplace_back(std::make_tuple(currentFile.string(), currentFile.extension().string(), fs.permissions(),
                                                  last_write_time(currentFile)));
            }
        }
    };
}
#endif
