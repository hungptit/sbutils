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
#include "TestData.hpp"

TEST(Filter, Positive) {
    {
        utils::filesystem::DoNothingPolicy f1;
        EXPECT_TRUE(f1.isValidExt(".git"));
        EXPECT_TRUE(f1.isValidStem(""));
    }

    {
        utils::filesystem::NormalPolicy filter;
        EXPECT_FALSE(filter.isValidExt(".git"));
        EXPECT_FALSE(filter.isValidStem("CMakeFiles"));
        EXPECT_TRUE(filter.isValidExt(".cpp"));
        EXPECT_TRUE(filter.isValidStem("foo"));
    }

    {
        utils::filesystem::MWPolicy filter;
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
void test_file_search(boost::filesystem::path &tmpDir) {
    using Container = std::vector<boost::filesystem::path>;
    utils::filesystem::SimpleVisitor<Container, Filter> visitor;
    Container searchFolders{tmpDir};
    utils::filesystem::dfs_file_search(searchFolders, visitor);
    auto results = visitor.getResults();
    std::cout << "== Search results ==\n";
    utils::print(results);
    EXPECT_EQ(results.size(), expectSize);
}

TEST(FileSearch, Positive) {
    utils::TemporaryDirectory tmpDir;
    TestData testData(tmpDir.getPath());
    auto tmpPath = tmpDir.getPath();
    test_file_search<utils::filesystem::DoNothingPolicy, 12>(tmpPath);
    test_file_search<utils::filesystem::NormalPolicy, 11>(tmpPath);
    test_file_search<utils::filesystem::MWPolicy, 11>(tmpPath);
}

TEST(FileSearc, Negative) {
    boost::filesystem::path tmpPath("foo");
    ASSERT_ANY_THROW(
        (test_file_search<utils::filesystem::DoNothingPolicy, 12>(tmpPath)));
}
