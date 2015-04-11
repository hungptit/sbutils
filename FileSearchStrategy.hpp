#ifndef FileSearchStrategy_hpp_
#define FileSearchStrategy_hpp_

#include "boost/filesystem.hpp"
#include <type_traits>
#include <algorithm>
#include "boost/regex.hpp"

namespace Tools {
    struct SearchAllFiles {
      static bool isValid(const boost::filesystem::file_status & fs) {
        return (fs.type() == boost::filesystem::regular_file);
      }
    };

    struct OwnerWriteFile {
        static bool isValid(const boost::filesystem::file_status & fs) {
            const auto fperm = fs.permissions();
            return ((fs.type() == boost::filesystem::regular_file) &&
                    (fperm & boost::filesystem::owner_write));
        }
    };

    struct OwnerReadFiles {
        static bool isValid(const boost::filesystem::file_status & fs) {
            auto fperm = fs.permissions();
            return ((fs.type() == boost::filesystem::regular_file) &&
                    (fperm & boost::filesystem::owner_read));
        }
    };

    
    template <typename Container>
    class SearchFileExtension {
      public:
        SearchFileExtension(const Container & supportedExts) : SupportedExtensions(supportedExts) {}
        
        bool isValid(const boost::filesystem::path & p) {
            const std::string fileExtension = p.extension().string();
            return std::find(SupportedExtensions.begin(), SupportedExtensions.end(), fileExtension) != SupportedExtensions.end();
        }
      private:
        Container SupportedExtensions;
    };


    class SearchFileName {
      public:
        SearchFileName (const boost::regex & exp) : Expression(exp) {}
        
        bool isValid(const boost::filesystem::path & p) {
            const std::string fileName = p.string();
            return boost::regex_match(fileName, Expression);
        }

      private:
        boost::regex Expression;

    };
}
#endif
