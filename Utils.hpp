#ifndef Utils_hpp_
#define Utils_hpp_

// Cereal
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

// Boost libraries
#include "boost/date_time/gregorian/gregorian.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/filesystem.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/uuid/uuid.hpp"
#include "boost/uuid/uuid_generators.hpp"
#include "boost/uuid/uuid_io.hpp"
#include <boost/iostreams/device/mapped_file.hpp> // for readLines

// Print out the content of std::tuple
// http://stackoverflow.com/questions/6245735/pretty-print-stdtuple
namespace {
    template <std::size_t...> struct int_ {};

    template <std::size_t N, std::size_t... Is>
    struct gen_seq : gen_seq<N - 1, N - 1, Is...> {};

    template <std::size_t... Is> struct gen_seq<0, Is...> : int_<Is...> {};

    template <class Ch, class Tr, class Tuple, std::size_t... Is>
    void print_tuple(std::basic_ostream<Ch, Tr> &output, Tuple const &t,
                     int_<Is...>) {
        using swallow = int[];
        (void)swallow{
            0,
            (void(output << (Is == 0 ? "" : ", ") << std::get<Is>(t)), 0)...};
    }

    template <class Ch, class Tr, class... Args>
    auto operator<<(std::basic_ostream<Ch, Tr> &output,
                    std::tuple<Args...> const &t)
        -> std::basic_ostream<Ch, Tr> & {
        print_tuple(output, t, gen_seq<sizeof...(Args)>());
        return output;
    }

    template <class Ch, class Tr, typename First, typename Second>
    auto operator<<(std::basic_ostream<Ch, Tr> &output,
                    std::pair<First, Second> const &t)
        -> std::basic_ostream<Ch, Tr> & {
        output << "(" << t.first << "," << t.second << ")";
        return output;
    }
}

namespace Utils {
    // Display utilities
    template <typename Data> void disp(Data &data, const std::string &message) {
        std::cout << message << "[ ";
        std::for_each(data.begin(), data.end(),
                      [](auto &val) { std::cout << val << " "; });
        std::cout << "]";
    }

    template <typename Data> void print(Data &data) {
        std::for_each(data.begin(), data.end(),
                      [](auto &val) { std::cout << val << "\n"; });
    }

    template <typename Container> void print(const Container &data) {
        for (auto const &val : data) {
            std::cout << val << "\n";
        }
    }

    // Define the default serilized formats
    typedef cereal::BinaryOutputArchive OArchive;
    typedef cereal::BinaryInputArchive IArchive;

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

    // Define the file information which is (path, stem, extension, permission,
    // time_stamp)
    typedef std::tuple<std::string, std::string, std::string, int, std::time_t>
        FileInfo;

    // Constant strings
    struct FileDatabaseInfo {
        static const std::string Database;
    };
    const std::string FileDatabaseInfo::Database = ".database";

    // Text file I/O
    std::vector<std::string> readLines(const std::string &dataFile) {
        boost::iostreams::mapped_file mmap(
            dataFile, boost::iostreams::mapped_file::readonly);
        auto begin = mmap.const_data();
        auto end = begin + mmap.size();

        std::vector<std::string> results;
        while (begin && begin != end) {
            auto currentPos = begin;
            while ((*currentPos != '\n') && (currentPos != end)) {
                ++currentPos;
            }
            results.emplace_back(std::string(begin, currentPos));
            begin = ++currentPos;
        }
        return results;
    }

    std::vector<std::string> readLines(const std::string &dataFile,
                                       size_t startLine, size_t stopLine) {
        boost::iostreams::mapped_file mmap(
            dataFile, boost::iostreams::mapped_file::readonly);
        auto begin = mmap.const_data();
        auto end = begin + mmap.size();

        std::vector<std::string> results;
        size_t linenum = 0;
        while (begin && begin != end) {
            auto currentPos = begin;
            while ((*currentPos != '\n') && (currentPos != end)) {
                ++currentPos;
            }
            ++linenum;
            if ((linenum >= startLine) && (linenum <= stopLine)) {
                results.emplace_back(std::string(begin, currentPos));
            }
            begin = ++currentPos;
        }
        return results;
    }

    std::vector<boost::filesystem::path>
    getFilesFromTxtFile(const boost::filesystem::path &dataFile,
                        bool verbose = false) {
        std::vector<boost::filesystem::path> results;
        for (auto aLine : readLines(dataFile.string())) {
            const auto aFile = boost::filesystem::path(aLine);
            boost::system::error_code errcode;
            if (boost::filesystem::is_regular_file(aFile, errcode)) {
                results.emplace_back(aFile);
            } else {
                if (verbose)
                    std::cout << aFile << ": " << errcode.message() << "\n";
            }
        }
        return results;
    }

    // Time related functionalities
    std::string getTimeStampString() {
        const boost::posix_time::ptime now =
            boost::posix_time::second_clock::local_time();
        return boost::posix_time::to_iso_string(now);
    }

    // File related utilities
    std::string normalize_path(const std::string &aPath) {
        auto pos = aPath.size() - 1;
        auto sepChar = boost::filesystem::path::preferred_separator;
        for (; pos != 0; --pos) {
            if (aPath[pos] != sepChar) break;
        }
        return aPath.substr(0, pos + 1);
    }

    bool isRegularFile(const std::string &str) {
        return boost::filesystem::is_regular_file(boost::filesystem::path(str));
    }

    bool isDirectory(const std::string &folderPath) {
        return boost::filesystem::is_directory(
            boost::filesystem::path(folderPath));
    }

    bool createDirectory(const std::string &folderPath) {
        const boost::filesystem::path folder(folderPath);
        return (boost::filesystem::is_directory(folder))
                   ? (true)
                   : (boost::filesystem::create_directories(folder));
    }

    bool remove(const std::string &folderName) {
        const boost::filesystem::path p(folderName);
        if (boost::filesystem::exists(p)) {
            if (boost::filesystem::is_directory(p)) {
                return boost::filesystem::remove_all(p);
            } else {
                return boost::filesystem::remove(p);
            }
        }
        return false;
    }

    const std::string getCurrentFolder() {
        return boost::filesystem::current_path().string();
    }

    std::string getAbslutePath(const std::string &pathName) {
        const boost::filesystem::path path(pathName);
        return boost::filesystem::canonical(path).string();
    }

    /**
     * This function will copy the content of the srcFolder to desFolder
     * recursively. Note that this function may throw.
     *
     * @param srcFolder
     * @param desFolder
     */
    void copyDir(const boost::filesystem::path &srcFolder,
                 const boost::filesystem::path &desFolder) {
        using namespace boost::filesystem;
        copy_directory(srcFolder, desFolder);
        recursive_directory_iterator endIter;
        recursive_directory_iterator dirIter(srcFolder);
        for (; dirIter != endIter; ++dirIter) {
            auto aFile = dirIter->path();
            if (is_directory(aFile)) {
                auto aPath = desFolder / aFile.filename();
                copyDir(aFile, aPath);
            } else {
                auto desFile = desFolder / aFile.filename();
                copy(aFile, desFile);
            }
        }
    }

    // files is a list of files with the relative paths.
    void copyFiles_rel(const std::vector<boost::filesystem::path> &files,
                       const boost::filesystem::path &srcDir,
                       const boost::filesystem::path &dstDir) {
        boost::system::error_code errcode;
        auto options = boost::filesystem::copy_option::overwrite_if_exists;
        for (auto aFile : files) {
            auto srcFile = srcDir / aFile;
            auto dstFile = dstDir / aFile;

            // TODO: Make sure that file properties are preserved, otherwise we
            // need to execute the copy command.
            boost::filesystem::copy_file(srcFile, dstFile, options);
        }
    }

    std::string findParent(const std::vector<std::string> &allKeys,
                           const std::string &aPath) {
        auto currentItem = std::find(allKeys.begin(), allKeys.end(), aPath);
        if (currentItem == allKeys.end()) {
            auto aFolder = boost::filesystem::canonical(aPath).parent_path();
            while (!aFolder.empty()) {
                auto currentItem =
                    std::find(allKeys.begin(), allKeys.end(), aFolder.string());
                if (currentItem != allKeys.end()) {
                    break;
                } else {
                    aFolder =
                        boost::filesystem::canonical(aFolder).parent_path();
                }
            }
            return aFolder.string();
        } else {
            return *currentItem;
        }
    }

    boost::filesystem::path
    getSandboxRoot(const boost::filesystem::path &aPath) {
        const boost::filesystem::path sbtool(".sbtools");
        auto sandbox = boost::filesystem::canonical(aPath);
        while (!sandbox.empty() &&
               !boost::filesystem::exists(sandbox / sbtool)) {
            sandbox = sandbox.parent_path();
        }
        return sandbox;
    }

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
        return std::make_tuple(folders, files);
    }

    // Explore a folder to a given level.
    // TODO: Need to rewrite this algorithm base on the BFS algorithms.
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
                        auto tmpPath = currentPath;
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
}
#endif
