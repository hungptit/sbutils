// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "boost/filesystem.hpp"
#include "gtest/gtest.h"
#include <array>
#include <fstream>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

#include "sbutils/FileSearch.hpp"
#include "sbutils/Print.hpp"
#include "sbutils/TemporaryDirectory.hpp"

#include "gtest/gtest.h"
#include "TestData.hpp"

TEST(Filter, Positive) {
    {
        sbutils::filesystem::DoNothingPolicy f1;
        EXPECT_TRUE(f1.isValidExt(".git"));
        EXPECT_TRUE(f1.isValidStem(""));
    }

    {
        sbutils::filesystem::NormalPolicy filter;
        EXPECT_FALSE(filter.isValidExt(".git"));
        EXPECT_FALSE(filter.isValidStem("CMakeFiles"));
        EXPECT_TRUE(filter.isValidExt(".cpp"));
        EXPECT_TRUE(filter.isValidStem("foo"));
    }

    {
        sbutils::filesystem::MWPolicy filter;
        EXPECT_FALSE(filter.isValidExt(".sbtools"));
        EXPECT_FALSE(filter.isValidStem("doxygen"));
        EXPECT_FALSE(filter.isValidStem("doc"));
        EXPECT_TRUE(filter.isValidStem("CMakeFiles"));
        EXPECT_TRUE(filter.isValidExt(".cpp"));
        EXPECT_TRUE(filter.isValidStem("foo"));
    }
}

TEST(FileDatabase, Positive) {
    sbutils::TemporaryDirectory tmpDir;
    TestData testData(tmpDir.getPath());
}

template <typename Filter, size_t expectSize>
void test_file_search(boost::filesystem::path &tmpDir) {
    using Container = std::vector<boost::filesystem::path>;
    sbutils::filesystem::SimpleVisitor<Container, Filter> visitor;
    Container searchFolders{tmpDir};
    sbutils::filesystem::dfs_file_search(searchFolders, visitor);
    auto results = visitor.getResults();
    std::cout << "== Search results ==\n";
    sbutils::print(results);
    EXPECT_EQ(results.size(), expectSize);
}

TEST(FileSearch, Positive) {
    sbutils::TemporaryDirectory tmpDir;
    TestData testData(tmpDir.getPath());
    auto tmpPath = tmpDir.getPath();
    test_file_search<sbutils::filesystem::DoNothingPolicy, 13>(tmpPath);
    test_file_search<sbutils::filesystem::NormalPolicy, 12>(tmpPath);
    test_file_search<sbutils::filesystem::MWPolicy, 12>(tmpPath);
}
