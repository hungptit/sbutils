#include "boost/filesystem.hpp"
#include "gtest/gtest.h"
#include <array>
#include <fstream>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

#include "utils/FileSearch.hpp"
#include "utils/Print.hpp"
#include "utils/TemporaryDirectory.hpp"

#include "gtest/gtest.h"

namespace {
    class TestData {
      public:
        using path = boost::filesystem::path;
        TestData(const path &currentPath) : TmpDir(currentPath) { init(); }
        ~TestData() {}

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
            auto dataFolder = createFolder(TmpDir, "data");
            createTestFile(dataFolder / path("data.mat"));

            auto gitFolder = createFolder(TmpDir, ".git");
            createTestFile(gitFolder / path("test.cpp"));

            createFolder(TmpDir, ".sbtools");
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

TEST(Filter, Positive) {
    {
        utils::DonotFilter filter;
        EXPECT_TRUE(filter.isValidExt(".git"));
        EXPECT_TRUE(filter.isValidStem(""));
    }

    {
        utils::NormalFilter filter;
        EXPECT_FALSE(filter.isValidExt(".git"));
        EXPECT_FALSE(filter.isValidStem("CMakeFiles"));
        EXPECT_TRUE(filter.isValidExt(".cpp"));
        EXPECT_TRUE(filter.isValidStem("foo"));
    }

    {
        utils::MWFilter filter;
        EXPECT_FALSE(filter.isValidExt(".sbtools"));
        EXPECT_FALSE(filter.isValidStem("doxygen"));
        EXPECT_FALSE(filter.isValidStem("doc"));
        EXPECT_TRUE(filter.isValidStem("CMakeFiles"));
        EXPECT_TRUE(filter.isValidExt(".cpp"));
        EXPECT_TRUE(filter.isValidStem("foo"));
    }
}

TEST(FileDatabase, Positive) {
  utils::TemporaryDirectory tmpDir;
  TestData testData(tmpDir.getPath());
}

template <typename Filter, size_t expectSize>
void test_file_search(utils::TemporaryDirectory &tmpDir) {
    using Container = std::vector<boost::filesystem::path>;
    utils::SimpleVisitor<Container, Filter> visitor;
    Container searchFolders{tmpDir.getPath()};
    utils::dfs_file_search<decltype(visitor), decltype(searchFolders)>(
        searchFolders, visitor);
    auto results = visitor.getResults();
    std::cout << "== Results ==\n";
    utils::print(results);
    EXPECT_EQ(results.size(), expectSize);
}

TEST(FileSearch, Positive) {
    utils::TemporaryDirectory tmpDir;
    TestData testData(tmpDir.getPath());
    test_file_search<utils::DonotFilter, 11>(tmpDir);
    test_file_search<utils::NormalFilter, 10>(tmpDir);
    test_file_search<utils::MWFilter, 11>(tmpDir);
}

// TEST(FileSearchStrategy, Positive) {
//     utils::TemporaryDirectory tmpDir;
//     {
//         TestData testData(tmpDir.getPath());
//         boost::filesystem::path aFile = tmpDir.getPath() /
//                                         boost::filesystem::path("src") /
//                                         boost::filesystem::path("test.cpp");
//         boost::filesystem::file_status fs = boost::filesystem::status(aFile);
//         {
//             utils::SearchAllFiles st;
//             EXPECT_TRUE(st.isValid(fs));
//         }

//         {
//             utils::OwnerReadFiles st;
//             EXPECT_TRUE(st.isValid(fs));
//         }

//         {
//             utils::OwnerWriteFile st;
//             EXPECT_TRUE(st.isValid(fs));
//         }
//     }

//     {
//         boost::filesystem::path aFile = tmpDir.getPath() /
//                                         boost::filesystem::path("data") /
//                                         boost::filesystem::path("data.mat");
//         boost::filesystem::file_status fs = boost::filesystem::status(aFile);
//         {
//             utils::OwnerReadFiles st;
//             EXPECT_TRUE(st.isValid(fs));
//         }

//         {
//             utils::OwnerWriteFile st;
//             EXPECT_TRUE(st.isValid(fs));
//         }
//     }

//     // File extension strategy
//     {
//         typedef std::vector<std::string> Map;
//         Map supportedExts = {".txt", ".dat", ".mat"};
//         std::sort(supportedExts.begin(), supportedExts.end());

//         utils::SearchFileExtension<std::vector<std::string>>
//         st(supportedExts);

//         {
//             boost::filesystem::path aFile =
//             boost::filesystem::path("data.mat");
//             EXPECT_TRUE(st.isValid(aFile));
//         }

//         {
//             boost::filesystem::path aFile =
//             boost::filesystem::path("data.foo");
//             EXPECT_FALSE(st.isValid(aFile));
//         }
//     }

//     // File name strategy
//     {
//         std::string fileName = "/this/is/a/test/file.foo";
//         boost::filesystem::path aFile = boost::filesystem::path(fileName);
//         const boost::regex expression1(".*this.*/file.foo");
//         const boost::regex expression2("file1");

//         {
//             utils::SearchFileName st(expression1);
//             EXPECT_TRUE(st.isValid(aFile));
//         }

//         {
//             utils::SearchFileName st(expression2);
//             EXPECT_FALSE(st.isValid(aFile));
//         }
//     }
// }

TEST(FileSearchDefault, Positive) {
    // utils::TemporaryDirectory tmpDir;
    //   TestData testData(tmpDir.getPath());
    // utils::FileFinder fSearch;
    // fSearch.search(tmpDir.getPath());
    // std::cout << "Search path: " << tmpDir.getPath().string() << std::endl;
    // fSearch.print();
    // auto &data = fSearch.getData();
    // EXPECT_TRUE(data.size() == 8);
    // if (!data.empty() > 0) {
    //     EXPECT_TRUE(
    //         std::get<0>(data[0]) ==
    //         (tmpDir.getPath() / boost::filesystem::path("bin")).string());
    //     EXPECT_TRUE(std::get<0>(data[1]) ==
    //                 (tmpDir.getPath() / boost::filesystem::path("bin") /
    //                  boost::filesystem::path("test")).string());
    // }
}

// TEST(FileSearchDefault, Negative) {
//     utils::FileFinder fSearch;
//     ASSERT_ANY_THROW(fSearch.search(boost::filesystem::path("foo")));
// }
