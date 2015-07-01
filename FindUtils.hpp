#ifndef FindUtils_hpp
#define FindUtils_hpp

#include <vector>
#include <string>
#include <tuple>

#include <boost/filesystem.hpp>
#include "boost/lexical_cast.hpp"
#include "boost/regex.hpp"

#include <cereal/types/memory.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/valarray.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/deque.hpp>
#include <cereal/types/forward_list.hpp>
#include <cereal/types/list.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/queue.hpp>
#include <cereal/types/set.hpp>
#include <cereal/types/stack.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/unordered_set.hpp>
#include <cereal/types/utility.hpp>
#include <cereal/types/tuple.hpp>
#include <cereal/types/bitset.hpp>
#include <cereal/types/complex.hpp>
#include <cereal/types/chrono.hpp>
#include <cereal/types/polymorphic.hpp>

#include <cereal/archives/binary.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/archives/xml.hpp>
#include <cereal/archives/json.hpp>

namespace Tools {
  // typedef cereal::JSONOutputArchive OArchive;
  // typedef cereal::JSONInputArchive IArchive;

  typedef cereal::BinaryOutputArchive OArchive;
  typedef cereal::BinaryInputArchive IArchive;
  
    typedef std::tuple<std::string, std::string, std::string> BasicFileInfo;

    // Implement Template Method design pattern using C++ template. This approach will minize the code duplication.
    class Finder {
      public:
        void search(const boost::filesystem::path &aPath) {
            boost::filesystem::recursive_directory_iterator endIter;
            boost::filesystem::recursive_directory_iterator dirIter(aPath);
            for (; dirIter != endIter; ++dirIter) {
                update(dirIter);
            }
        }
        
      protected:
        virtual void update(boost::filesystem::recursive_directory_iterator &dirIter) = 0;
    };

    // Build a simple file database which containts file paths and extensions
    template <typename Base, typename T> class BuildFileDatabase; // Base class

    template <typename Base> class BuildFileDatabase<Base, BasicFileInfo> : public Base {
      public:
         std::vector<BasicFileInfo> &getData() { return Data; }

      protected:
        void update(boost::filesystem::recursive_directory_iterator &dirIter) {

            const boost::filesystem::file_status fs = dirIter->status();
            if (fs.type() == boost::filesystem::regular_file) {
                auto const aPath = dirIter->path();
                Data.emplace_back(std::make_tuple(aPath.string(), aPath.stem().string(), aPath.extension().string()));
            }
        }

      private:
        std::vector<BasicFileInfo> Data;
    };


    typedef std::tuple<std::string, std::string, boost::filesystem::perms, std::time_t> FileInfo;

    // template <typename Base> class BuildFileDatabase<Base, std::string> : public Base {
    //   public:
    //     const decltype(Data) &getData() const { return Data; }

    //   protected:
    //     void update(boost::filesystem::recursive_directory_iterator &dirIter) {

    //         const boost::filesystem::file_status fs = dirIter->status();
    //         if (fs.type() == boost::filesystem::regular_file) {
    //             auto const currentFile = dirIter->path();
    //             const BasicFileInfo t = {currentFile.string(), currentFile.extension().string()};
    //             Data.emplace_back(t);
    //         }
    //     }

    //   private:
    //     std::vector<std::string> Data;
    // };


    template <typename OArchive, typename Container> void save(const Container &data, std::ostringstream &os) {
        OArchive oar(os);
        oar(cereal::make_nvp("data", data));
    }

    template <typename OArchive, typename Container> void load(Container &data, std::istringstream &is) {
        OArchive iar(is);
        iar(data);
    }
}
#endif
