#include <iostream>
#include <fstream>
#include "gtest/gtest.h"
#include <string>
#include <vector>
#include <array>
#include <tuple>
#include "boost/filesystem.hpp"
#include "utils/Utils.hpp"

class TestData {
  public:
    TestData(const boost::filesystem::path &currentPath) : TmpDir(currentPath) {
        init();
    }
    ~TestData() {}

  private:
    boost::filesystem::path TmpDir;

    void createTestFile(const boost::filesystem::path &aFile) {
        std::ofstream outfile(aFile.string());
        outfile << "Fake data for testing.\n";
        outfile.close();
        if (boost::filesystem::exists(aFile)) {
            std::cout << aFile.string() << " is created" << std::endl;
        } else {
            std::cout << "Cannot create " << aFile.string() << std::endl;
        }
    }

    void init() {
        {
            // Create data folder
            boost::filesystem::path dataFolder =
                TmpDir / boost::filesystem::path("data");
            boost::filesystem::create_directories(dataFolder);

            // Create a data file
            auto aDataFile = dataFolder / boost::filesystem::path("data.mat");
            createTestFile(aDataFile);
            boost::filesystem::permissions(aDataFile,
                                           boost::filesystem::owner_read |
                                               boost::filesystem::owner_write);
        }

        {
            // Create src folder
            boost::filesystem::path srcFolder =
                TmpDir / boost::filesystem::path("src");
            boost::filesystem::create_directories(srcFolder);
            auto aSrcFile = srcFolder / boost::filesystem::path("test.cpp");
            createTestFile(aSrcFile);
            boost::filesystem::permissions(aSrcFile,
                                           boost::filesystem::owner_write |
                                               boost::filesystem::owner_read);
            createTestFile(srcFolder / boost::filesystem::path("read.cpp"));
            createTestFile(srcFolder / boost::filesystem::path("write.cpp"));
        }

        {
            // Create bin folder
            boost::filesystem::path binFolder =
                TmpDir / boost::filesystem::path("bin");
            boost::filesystem::create_directories(binFolder);
            auto aBinFile = binFolder / boost::filesystem::path("test");
            createTestFile(aBinFile);
            boost::filesystem::permissions(aBinFile,
                                           boost::filesystem::owner_exe |
                                               boost::filesystem::group_exe |
                                               boost::filesystem::owner_read |
                                               boost::filesystem::group_read);
        }
    }
};

TEST(FileSearchStrategy, Positive) {
    Tools::TemporaryDirectory tmpDir;
    {
        TestData testData(tmpDir.getPath());
        boost::filesystem::path aFile = tmpDir.getPath() /
                                        boost::filesystem::path("src") /
                                        boost::filesystem::path("test.cpp");
        boost::filesystem::file_status fs = boost::filesystem::status(aFile);
        {
            Tools::SearchAllFiles st;
            EXPECT_TRUE(st.isValid(fs));
        }

        {
            Tools::OwnerReadFiles st;
            EXPECT_TRUE(st.isValid(fs));
        }

        {
            Tools::OwnerWriteFile st;
            EXPECT_TRUE(st.isValid(fs));
        }
    }

    {
        boost::filesystem::path aFile = tmpDir.getPath() /
                                        boost::filesystem::path("data") /
                                        boost::filesystem::path("data.mat");
        boost::filesystem::file_status fs = boost::filesystem::status(aFile);
        {
            Tools::OwnerReadFiles st;
            EXPECT_TRUE(st.isValid(fs));
        }

        {
            Tools::OwnerWriteFile st;
            EXPECT_TRUE(st.isValid(fs));
        }
    }

    // File extension strategy
    {
        typedef std::vector<std::string> Map;
        Map supportedExts = {".txt", ".dat", ".mat"};
        std::sort(supportedExts.begin(), supportedExts.end());

        Tools::SearchFileExtension<std::vector<std::string>> st(supportedExts);

        {
            boost::filesystem::path aFile = boost::filesystem::path("data.mat");
            EXPECT_TRUE(st.isValid(aFile));
        }

        {
            boost::filesystem::path aFile = boost::filesystem::path("data.foo");
            EXPECT_FALSE(st.isValid(aFile));
        }
    }

    // File name strategy
    {
        std::string fileName = "/this/is/a/test/file.foo";
        boost::filesystem::path aFile = boost::filesystem::path(fileName);
        const boost::regex expression1(".*this.*/file.foo");
        const boost::regex expression2("file1");

        {
            Tools::SearchFileName st(expression1);
            EXPECT_TRUE(st.isValid(aFile));
        }

        {
            Tools::SearchFileName st(expression2);
            EXPECT_FALSE(st.isValid(aFile));
        }
    }
}

TEST(FileSearchDefault, Positive) {
    Tools::TemporaryDirectory tmpDir;
    TestData testData(tmpDir.getPath());
    Tools::FileFinder fSearch;
    fSearch.search(tmpDir.getPath());
    std::cout << "Search path: " << tmpDir.getPath().string() << std::endl;
    fSearch.print();
    auto &data = fSearch.getData();
    EXPECT_TRUE(data.size() == 8);
    if (!data.empty() > 0) {
        EXPECT_TRUE(
            data[0].first ==
            (tmpDir.getPath() / boost::filesystem::path("bin")).string());
        EXPECT_TRUE(data[1].first ==
                    (tmpDir.getPath() / boost::filesystem::path("bin") /
                     boost::filesystem::path("test")).string());
    }
}

TEST(FileSearchDefault, Negative) {
    Tools::FileFinder fSearch;
    ASSERT_ANY_THROW(fSearch.search(boost::filesystem::path("foo")));
}
