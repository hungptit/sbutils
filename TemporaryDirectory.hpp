#ifndef TemporaryDirectory_hpp_
#define TemporaryDirectory_hpp_

#include "boost/filesystem.hpp"
#include "boost/uuid/uuid.hpp"
#include "boost/uuid/uuid_generators.hpp"
#include "boost/uuid/uuid_io.hpp"

// Random unique string
namespace {
    const std::string getUniqueString() {
        return boost::lexical_cast<std::string>(
            boost::uuids::random_generator()());
    }
}

class TemporaryDirectory {
  public:
    TemporaryDirectory() {
        CurrentDir = boost::filesystem::temp_directory_path() /
                     boost::filesystem::unique_path("%%%%-%%%%-%%%%-%%%%");
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

#endif
