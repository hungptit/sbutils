#ifndef FileSystemUtilities_hpp_
#define FileSystemUtilities_hpp_
#include "boost/filesystem.hpp"
#include <string>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "boost/lexical_cast.hpp"
namespace Tools {
bool isRegularFile(const std::string &str) {
    return boost::filesystem::is_regular_file(boost::filesystem::path(str));
}

bool isDirectory(const std::string &folderPath) {
    return boost::filesystem::is_directory(boost::filesystem::path(folderPath));
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

const std::string getUniqueString() {
    return boost::lexical_cast<std::string>(boost::uuids::random_generator()());
}

class TemporaryDirectory {
  public:
    TemporaryDirectory() {
        CurrentDir = boost::filesystem::temp_directory_path() /
                     boost::filesystem::path(getUniqueString());
        boost::filesystem::create_directories(CurrentDir);
    }

    TemporaryDirectory(const std::string &parentDir) {
        CurrentDir = boost::filesystem::path(parentDir) /
                     boost::filesystem::path(getUniqueString());
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
