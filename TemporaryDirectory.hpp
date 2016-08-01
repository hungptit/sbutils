#ifndef TemporaryDirectory_hpp_
#define TemporaryDirectory_hpp_

#include "boost/filesystem.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/uuid/uuid.hpp"
#include "boost/uuid/uuid_generators.hpp"
#include "boost/uuid/uuid_io.hpp"

namespace utils {
    const std::string getUniqueString() {
        return boost::lexical_cast<std::string>(
            boost::uuids::random_generator()());
    }

    class TemporaryDirectory {
      public:
        explicit TemporaryDirectory() {
            CurrentDir = boost::filesystem::temp_directory_path() /
                         boost::filesystem::unique_path("%%%%-%%%%-%%%%-%%%%");
            boost::filesystem::create_directories(CurrentDir);
        }

        explicit TemporaryDirectory(const std::string &parentDir) {
            CurrentDir = boost::filesystem::path(parentDir) /
                         boost::filesystem::path(getUniqueString());
            boost::filesystem::create_directories(CurrentDir);
        }

        TemporaryDirectory(const TemporaryDirectory &tmpDir) = delete;

        TemporaryDirectory(TemporaryDirectory &&tmpDir) noexcept
            : CurrentDir(std::move(tmpDir.CurrentDir)) {}

        ~TemporaryDirectory() {
            if (boost::filesystem::exists(CurrentDir)) {
                boost::filesystem::remove_all(CurrentDir);
            }
        }

        const boost::filesystem::path &getPath() { return CurrentDir; }

      private:
        boost::filesystem::path CurrentDir;
    };
}

#endif
