#ifndef FindUtils_hpp
#define FindUtils_hpp

#include <string>
#include <tuple>
#include <vector>

#include "boost/lexical_cast.hpp"
#include "boost/regex.hpp"
#include <boost/filesystem.hpp>

#include <cereal/types/array.hpp>
#include <cereal/types/bitset.hpp>
#include <cereal/types/chrono.hpp>
#include <cereal/types/complex.hpp>
#include <cereal/types/deque.hpp>
#include <cereal/types/forward_list.hpp>
#include <cereal/types/list.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/types/queue.hpp>
#include <cereal/types/set.hpp>
#include <cereal/types/stack.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/tuple.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/unordered_set.hpp>
#include <cereal/types/utility.hpp>
#include <cereal/types/valarray.hpp>
#include <cereal/types/vector.hpp>

#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/archives/xml.hpp>

namespace Tools {
    typedef cereal::BinaryOutputArchive OArchive;
    typedef cereal::BinaryInputArchive IArchive;

    typedef std::tuple<std::string, std::string, std::string> BasicFileInfo;
    typedef std::tuple<std::string, std::string, std::string, int, std::time_t>
        EditedFileInfo;

    struct FileDatabaseInfo {
        static const std::string Database;
    };
    const std::string FileDatabaseInfo::Database = ".database";

    // Implement Template Method design pattern using C++ template. This
    // approach will minize the code duplication.
    class Finder {
      public:
        typedef boost::filesystem::path Path;
        typedef boost::filesystem::recursive_directory_iterator iter_type;
        void search(const Path &aPath) {
            iter_type endIter, dirIter(aPath);
            for (; dirIter != endIter; ++dirIter) {
                update(dirIter);
            }
        }

      protected:
        virtual void update(iter_type &dirIter) = 0;
    };

    // Build a simple file database which containts file paths and extensions
    template <typename Base, typename T> class BuildFileDatabase; // Base class

    template <typename Base>
    class BuildFileDatabase<Base, BasicFileInfo> : public Base {
      public:
        typedef BasicFileInfo value_type;
        std::vector<value_type> &getData() { return Data; }

      protected:
        void update(typename Base::iter_type &dirIter) {

            const boost::filesystem::file_status fs = dirIter->status();
            if (fs.type() == boost::filesystem::regular_file) {
                auto const aPath = dirIter->path();
                Data.emplace_back(std::make_tuple(aPath.string(),
                                                  aPath.stem().string(),
                                                  aPath.extension().string()));
            }
        }

      private:
        std::vector<value_type> Data;
    };

    // Build an edited file database which contains file paths, stems,
    // extensions, permisisons, and last write times.
    template <typename Base>
    class BuildFileDatabase<Base, EditedFileInfo> : public Base {
      public:
        typedef EditedFileInfo value_type;
        std::vector<value_type> &getData() { return Data; }

      protected:
        void update(typename Base::iter_type &dirIter) {
            const boost::filesystem::file_status fs = dirIter->status();
            if (fs.type() == boost::filesystem::regular_file) {
                auto const aPath = dirIter->path();
                Data.emplace_back(std::make_tuple(
                    aPath.string(), aPath.stem().string(),
                    aPath.extension().string(), fs.permissions(),
                    boost::filesystem::last_write_time(aPath)));
            }
        }

      private:
        std::vector<value_type> Data;
    };

    // Find files
    template <typename Base> class FindFiles : public Base {
      public:
        typedef BasicFileInfo value_type;

        FindFiles(const std::vector<std::string> &supportedExts)
            : Extensions(supportedExts) {}
        std::vector<value_type> &getData() { return Data; }

      protected:
        void update(boost::filesystem::recursive_directory_iterator &dirIter) {
            const boost::filesystem::file_status fs = dirIter->status();
            if (fs.type() == boost::filesystem::regular_file) {
                auto const aPath = dirIter->path();
                const auto extension = aPath.extension().string();
                bool isValid = Extensions.empty() ||
                               (std::find(Extensions.begin(), Extensions.end(),
                                          extension) != Extensions.end());
                if (isValid) {
                    Data.emplace_back(std::make_tuple(
                        aPath.parent_path().string(), aPath.stem().string(),
                        aPath.extension().string()));
                }
            }
        }

      private:
        std::vector<std::string> Extensions;
        std::vector<value_type> Data;
    };

    // Find edited files
    template <typename Base> class FindEditedFiles : public Base {
      public:
        typedef EditedFileInfo value_type;

        FindEditedFiles(const std::vector<std::string> &supportedExts)
            : Extensions(supportedExts) {}
        std::vector<value_type> &getData() { return Data; }

      protected:
        void update(boost::filesystem::recursive_directory_iterator &dirIter) {
            const boost::filesystem::file_status fs = dirIter->status();
            if (fs.type() == boost::filesystem::regular_file) {
                const auto fperm = fs.permissions();
                if (fperm & boost::filesystem::owner_write) {
                    auto const aPath = dirIter->path();
                    const auto extension = aPath.extension().string();
                    bool isValid =
                        Extensions.empty() ||
                        (std::find(Extensions.begin(), Extensions.end(),
                                   extension) != Extensions.end());
                    if (isValid) {
                        Data.emplace_back(std::make_tuple(
                            aPath.string(), aPath.stem().string(),
                            aPath.extension().string(), fs.permissions(),
                            boost::filesystem::last_write_time(aPath)));
                    }
                }
            }
        }

      private:
        std::vector<std::string> Extensions;
        std::vector<value_type> Data;
    };

    boost::filesystem::path getPath(const boost::filesystem::path &aPath,
                                    bool useRelativePath) {
        if (useRelativePath) {
            auto tmpPath = aPath.stem();
            tmpPath += aPath.extension();
            return tmpPath;
        } else {
            return boost::filesystem::canonical(aPath);
        }
    }

    std::tuple<std::vector<boost::filesystem::path>,
               std::vector<boost::filesystem::path>>
    exploreFolderAtRootLevel(const boost::filesystem::path &aPath,
                             bool useRelativePath) {
        std::vector<boost::filesystem::path> files;
        std::vector<boost::filesystem::path> folders;

        // Explore a given folder at the root level.
        boost::filesystem::directory_iterator endIter;
        boost::filesystem::directory_iterator dirIter(aPath);
        for (; dirIter != endIter; ++dirIter) {
            auto currentPath = dirIter->path();
            if (boost::filesystem::is_directory(currentPath)) {
                folders.push_back(getPath(currentPath, useRelativePath));
            } else if (boost::filesystem::is_regular_file(currentPath)) {
                files.push_back(getPath(currentPath, useRelativePath));
            }
        }

        // Return
        return std::make_tuple(folders, files);
    }

    // Explore the given folder to a given level
    std::tuple<std::vector<boost::filesystem::path>,
               std::vector<boost::filesystem::path>>
    exploreFolders(size_t level, const boost::filesystem::path &rootFolder,
                   bool useRelativePath = false) {
        auto results = exploreFolderAtRootLevel(rootFolder, useRelativePath);
        std::vector<boost::filesystem::path> files = std::get<1>(results);
        std::vector<boost::filesystem::path> folders = std::get<0>(results);
        size_t counter = 1;

        // This code does not make any assumtion about the input path.
        while (counter < level) {
            decltype(folders) nextLevel;
            for (auto const &aPath : folders) {
                boost::filesystem::directory_iterator endIter;
                boost::filesystem::directory_iterator dirIter(aPath);
                for (; dirIter != endIter; ++dirIter) {
                    auto currentPath = dirIter->path();
                    if (boost::filesystem::is_directory(currentPath)) {
                        nextLevel.push_back(currentPath);
                    } else if (boost::filesystem::is_regular_file(
                                   currentPath)) {
                        files.push_back(currentPath);
                    }
                }
            }

            if (nextLevel.empty()) {
                break;
            } else {
                folders.reserve(nextLevel.size());
                // Move content of nextLevel to folders then clear nextLevel
                // content.
                folders = std::move(nextLevel);
                counter++;
            }
        }
        return std::make_tuple(folders, files);
    }

    // Load/save data from the string stream using Cereal.
    typedef cereal::BinaryOutputArchive DefaultOArchive;
    typedef cereal::BinaryInputArchive DefaultIArchive;

    template <typename OArchive, typename Container>
    void save(const Container &data, std::ostringstream &os) {
        OArchive oar(os);
        oar(cereal::make_nvp("data", data));
    }

    template <typename OArchive, typename Container>
    void load(Container &data, std::istringstream &is) {
        OArchive iar(is);
        iar(data);
    }
}
#endif
