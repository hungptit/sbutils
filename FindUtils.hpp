#ifndef FindUtils_hpp
#define FindUtils_hpp

#include <vector>
#include <string>

#include <boost/filesystem.hpp>
#include "boost/lexical_cast.hpp"
#include "boost/regex.hpp"

#include "cereal/archives/json.hpp"
#include "cereal/archives/binary.hpp"
#include "cereal/archives/xml.hpp"

namespace Tools {
    struct BasicFileInfo {
        std::string Path;
        std::string Extension;

        template <class Archive> void serialize(Archive &archive) { archive(CEREAL_NVP(Path), CEREAL_NVP(Extension)); }
    };

    struct ExtendedFileInfo {
        std::string Path;
        std::string Extension;
        int Permission;
        std::string LastWriteTime;

        template <class Archive> void serialize(Archive &archive) {
            archive(CEREAL_NVP(Path), CEREAL_NVP(Extension), CEREAL_NVP(Permission), CEREAL_NVP(LastWriteTime));
        }
    };

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
    template <typename Base, typename T> class BuildFileDatabase;


    template <typename Base> class BuildFileDatabase<Base, std::string> : public Base {
      public:
        const std::vector<std::string> &getData() const { return Data; }
        template <class Archive> void serialize(Archive &archive) { archive(CEREAL_NVP(Data)); }

      protected:
        void update(boost::filesystem::recursive_directory_iterator &dirIter) {

            const boost::filesystem::file_status fs = dirIter->status();
            if (fs.type() == boost::filesystem::regular_file) {
                Data.emplace_back(dirIter->path().string());
            }
        }

      private:
        std::vector<std::string> Data;
    };


    template <typename Base> class BuildFileDatabase<Base, BasicFileInfo> : public Base {
      public:
        const std::vector<BasicFileInfo> &getData() const { return Data; }

      protected:
        void update(boost::filesystem::recursive_directory_iterator &dirIter) {

            const boost::filesystem::file_status fs = dirIter->status();
            if (fs.type() == boost::filesystem::regular_file) {
                auto const currentFile = dirIter->path();
                const BasicFileInfo t = {currentFile.string(), currentFile.extension().string()};
                Data.emplace_back(t);
            }
        }

      private:
        std::vector<BasicFileInfo> Data;
    };

    
    // Display the searched information
    template <typename OutputArchive, typename Container> void disp(Container &data) {
        std::stringstream ss;
        OutputArchive oarchive(ss);
        for (auto const &val : data) {
            oarchive(val);
        }
        std::cout << ss.str() << "\n";
    }
}
#endif
