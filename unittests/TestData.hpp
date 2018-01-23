#pragma once

#include "sbutils/TemporaryDirectory.hpp"

namespace {
    class TestData {
      public:
        using path = boost::filesystem::path;
        explicit TestData(const path &currentPath) : TmpDir(currentPath) {
            init();
        }

      private:
        boost::filesystem::path TmpDir;

        void createTestFile(const path &aFile) {
            std::ofstream outfile(aFile.string());
            outfile << "Fake data for testing.\n";
            outfile.close();
            if (boost::filesystem::exists(aFile)) {
                std::cout << aFile.string() << " is created" << std::endl;
            } else {
                std::cout << "Cannot create " << aFile.string() << std::endl;
            }
        }

        path createFolder(const path &rootFolder, const path &aPath) {
            auto fullPath = rootFolder / aPath;
            boost::filesystem::create_directories(fullPath);
            return fullPath;
        }

        void init() {
            createTestFile(TmpDir / path("README.md"));

            auto dataFolder = createFolder(TmpDir, "data");
            createTestFile(dataFolder / path("data.mat"));

            auto gitFolder = createFolder(TmpDir, ".git");
            createTestFile(gitFolder / path("test.cpp"));

            auto sbtoolsFolder = createFolder(TmpDir, ".sbtools");
            createTestFile(sbtoolsFolder / path("foo.p"));

            createFolder(TmpDir, "CMakeFiles");

            auto srcFolder = createFolder(TmpDir, "src");
            createTestFile(srcFolder / path("test.cpp"));
            createTestFile(srcFolder / path("read.cpp"));
            createTestFile(srcFolder / path("write.cpp"));
            createTestFile(srcFolder / path("write.p"));
            createTestFile(srcFolder / path("write.txt"));
            createTestFile(srcFolder / path("foo.m"));
            createTestFile(srcFolder / path("foo.mat"));
            createTestFile(srcFolder / path("foo.p"));

            auto binFolder = createFolder(TmpDir, "bin");
            createTestFile(binFolder / path("test"));

            auto docFolder = createFolder(TmpDir, "doc");
        }
    };
}
