#pragma once

#include "sbutils/TemporaryDirectory.hpp"

namespace {
    class TestData {
      public:
        using path = boost::filesystem::path;
        explicit TestData(const path &currentPath, const bool verbose = false) : TmpDir(currentPath) { init(verbose); }

      private:
        boost::filesystem::path TmpDir;

        void createTestFile(const path &aFile, const bool verbose = false) {
            std::ofstream outfile(aFile.string());
            outfile << "Fake data for testing.\n";
            outfile.close();
            if (boost::filesystem::exists(aFile)) {
                if (verbose) std::cout << aFile.string() << " is created" << std::endl;
            } else {
                std::cerr << "Cannot create " << aFile.string() << std::endl;
            }
        }

        path createFolder(const path &rootFolder, const path &aPath) {
            auto fullPath = rootFolder / aPath;
            boost::filesystem::create_directories(fullPath);
            return fullPath;
        }

        void init(const bool verbose) {
            createTestFile(TmpDir / path("README.md"));

            auto dataFolder = createFolder(TmpDir, "data");
            createTestFile(dataFolder / path("data.mat"));

            auto gitFolder = createFolder(TmpDir, ".git");
            createTestFile(gitFolder / path("test.cpp"));

            auto sbtoolsFolder = createFolder(TmpDir, ".subversion");
            createTestFile(sbtoolsFolder / path("foo.p"), verbose);

            createFolder(TmpDir, "CMakeFiles");

            auto srcFolder = createFolder(TmpDir, "src");
            createTestFile(srcFolder / path("test.cpp"), verbose);
            createTestFile(srcFolder / path("read.cpp"), verbose);
            createTestFile(srcFolder / path("write.cpp"), verbose);
            createTestFile(srcFolder / path("write.p"), verbose);
            createTestFile(srcFolder / path("write.txt"), verbose);
            createTestFile(srcFolder / path("foo.m"), verbose);
            createTestFile(srcFolder / path("foo.mat"), verbose);
            createTestFile(srcFolder / path("foo.p"), verbose);

            auto binFolder = createFolder(TmpDir, "bin");
            createTestFile(binFolder / path("test"), verbose);

            auto docFolder = createFolder(TmpDir, "doc");
        }
    };
} // namespace
