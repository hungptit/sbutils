#ifndef FileSystemUtilities_hpp_
#define FileSystemUtilities_hpp_
#include "boost/filesystem.hpp"
#include <string>
namespace Tools {
    bool createDirectory(const std::string &folderName);
    bool remove(const std::string &folderName);
    bool isRegularFile(const std::string &str);
    bool isDirectory(const std::string &str);
    const std::string getCurrentFolder();
    std::string getAbslutePath(const std::string &path);

    // Create a temporary folder for testing.
    class TemporaryDirectory {
      public:
        TemporaryDirectory()
            : CurrentDir(std::tmpnam(const_cast<char *>(
                                         boost::filesystem::temp_directory_path().string().c_str()))) {
            boost::filesystem::create_directories(CurrentDir);
        }
        
        ~TemporaryDirectory() {
            if (boost::filesystem::exists(CurrentDir)) {
                boost::filesystem::remove_all(CurrentDir);
            }
        }
        
        const boost::filesystem::path &getPath() { return CurrentDir; }

      private:
        boost::filesystem::path CurrentDir;
    };

#include "private/FileSystemUtilities.cpp"
}
#endif
