#ifndef FileSystemUtilities_hpp_
#define FileSystemUtilities_hpp_
#include "boost/filesystem.hpp"
#include <string>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "boost/lexical_cast.hpp"
namespace Tools {
  bool createDirectory(const std::string &folderName);
  bool remove(const std::string &folderName);
  bool isRegularFile(const std::string &str);
  bool isDirectory(const std::string &str);
  const std::string getCurrentFolder();
  std::string getAbslutePath(const std::string &path);
  std::string getUniqueString();
  // Create a temporary folder for testing.
  class TemporaryDirectory {
  public:
    TemporaryDirectory() {
      CurrentDir = boost::filesystem::temp_directory_path() / boost::filesystem::path(getUniqueString());
      boost::filesystem::create_directories(CurrentDir);
    }


    TemporaryDirectory(const std::string & parentDir) {
      CurrentDir = boost::filesystem::path(parentDir) / boost::filesystem::path(getUniqueString());
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
