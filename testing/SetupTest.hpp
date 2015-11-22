#include "gtest/gtest.h"
#include <array>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <vector>

#include "utils/BFSFileSearch.hpp"
#include "utils/DFSFileSearch.hpp"
#include "utils/FileSearch.hpp"
#include "utils/LevelDBIO.hpp"
#include "utils/TemporaryDirectory.hpp"
#include "utils/Timer.hpp"
#include "utils/Utils.hpp"

namespace {
    class SetupTestDirectory {
      public:
        using path = boost::filesystem::path;
        SetupTestDirectory()
            : OriginalPath(boost::filesystem::current_path()),
              CurrentPath(boost::filesystem::temp_directory_path() /
                          path("utils")) {
            // Create fake test folder hierarchy
            boost::filesystem::create_directories(CurrentPath);
            createFile(CurrentPath, "fooo.txt");

            // 1st level folders
            path f11 = CurrentPath / "foo";
            path f12 = CurrentPath / "boo";
            boost::filesystem::create_directories(f11);
            boost::filesystem::create_directories(f12);
            createFile(f12, "fooo.cpp");
            createFile(f12, "fooo.hpp");
            createFile(f12, "CMakeLists.txt");
            
            // 1st level folders
            path f21 = f11 / "1";
            path f22 = f11 / "boo";
            boost::filesystem::create_directories(f21);
            boost::filesystem::create_directories(f22);
            createFile(f21, "fooo.so");
            createFile(f21, "fooo.a");
        }

        ~SetupTestDirectory() { boost::filesystem::remove_all(CurrentPath); }

      void createFile(const path & parent, const path &aFile) {
        path aPath(parent / aFile);
        std::ofstream myfile;
        myfile.open(aPath.string());
        myfile << aFile << "\n";
        }

        const path getOriginalPath() const { return OriginalPath; }
        const path getCurrentPath() const { return CurrentPath; }

      private:
        path OriginalPath;
        path CurrentPath;
    };
}
