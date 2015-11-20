#ifndef Utils_hpp_
#define Utils_hpp_

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

#include "boost/filesystem.hpp"
#include "boost/uuid/uuid.hpp"
#include "boost/uuid/uuid_generators.hpp"
#include "boost/uuid/uuid_io.hpp"

#include "BasicUtilities.hpp"
#include "FileSystemUtilities.hpp"
#include "TimeUtilities.hpp"

namespace Utils {
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
}

#endif
