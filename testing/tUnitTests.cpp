#include "gtest/gtest.h"
#include <string>
#include <map>
#include <vector>
#include <array>
#include <tuple>
#include <iostream>

#include "utils/BFSFileSearch.hpp"
#include "utils/DFSFileSearch.hpp"
#include "utils/FileSearch.hpp"
#include "utils/LevelDBIO.hpp"
#include "utils/Timer.hpp"
#include "utils/TemporaryDirectory.hpp"
#include "utils/Utils.hpp"

#include "SetupTest.hpp"

TEST(Display_Functions, Positive) {
    {
        std::vector<int> data = {1, 2, 3, 4, 5, 6};
        Utils::disp(data, "data: ");
    }

    {
        std::map<int, std::string> data = {{1, "a"}, {2, "b"}, {3, "c"}};
        Utils::disp(data, "data: ");
    }

    {
        std::cout << "Current time: " << Utils::getTimeStampString()
                  << std::endl;
    }

    std::cout << boost::filesystem::unique_path("%%%%-%%%%-%%%%-%%%%")
              << std::endl;
}

TEST(FileSystemUtilities, Positive) {
    const boost::filesystem::path aFolder =
        boost::filesystem::temp_directory_path() /
        boost::filesystem::path("test");
    const std::string folderName = aFolder.string();
    EXPECT_TRUE(Utils::createDirectory(folderName));
    EXPECT_TRUE(Utils::isDirectory(folderName));
    EXPECT_TRUE(Utils::getAbslutePath(folderName) == folderName);
    EXPECT_TRUE(Utils::getAbslutePath("./") == Utils::getCurrentFolder());
    EXPECT_TRUE(Utils::remove(folderName));
}

TEST(FileSystemUtilities, Negative) {
    std::string folderName = "/usr/test";
    ASSERT_ANY_THROW(Utils::createDirectory(folderName));
    EXPECT_FALSE(Utils::remove(folderName));
    EXPECT_FALSE(Utils::isRegularFile(folderName));
    EXPECT_FALSE(Utils::isDirectory("tUnitTests.cpp"));
}

TEST(TemporaryDirectory, Positive) {
    TemporaryDirectory tmpDir;
    std::cout << tmpDir.getPath() << std::endl;
    EXPECT_TRUE(boost::filesystem::exists(tmpDir.getPath()));
}


TEST(ExporeFolderRootLevel, Positive) {
    {
        TemporaryDirectory tmpDir;
        auto results = Utils::exploreFolderAtRootLevel(tmpDir.getPath(), 0);
        fmt::print("Folders:\n");
        for (auto item : std::get<0>(results)) {
            std::cout << item << "\n";
        }
        fmt::print("Files:\n");
        for (auto item : std::get<1>(results)) {
            std::cout << item << "\n";
        }
    }
}

TEST(FileSearch, Positive) {
  SetupTestDirectory setup;
  auto aPath = setup.getCurrentPath();

  // Search for files using DFS
  Utils::FileSearchBase<Utils::DFSFileSearchBase> dfs_finder;
  std::cout << "Number of files: " << dfs_finder.search(aPath) << std::endl;

    // Search for files using BFS
  Utils::FileSearchBase<Utils::BFSFileSearchBase> bfs_finder;
  std::cout << "Number of files: " << bfs_finder.search(aPath) << std::endl;

  // The number of files obtained using two algorithms should be the same
  EXPECT_TRUE(dfs_finder.getData().size() == bfs_finder.getData().size());
}
