#include "gtest/gtest.h"
#include <array>
#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <vector>

#include "utils/BFSFileSearch.hpp"
#include "utils/DFSFileSearch.hpp"
#include "utils/FileSearch.hpp"
#include "utils/FileUtils.hpp"
#include "utils/LevelDBIO.hpp"
#include "utils/Print.hpp"
#include "utils/TemporaryDirectory.hpp"
#include "utils/Timer.hpp"

#include "SetupTest.hpp"

using path = boost::filesystem::path;

TEST(Display_Functions, Positive) {
    {
        std::vector<int> data = {1, 2, 3, 4, 5, 6};
        utils::disp(data, "data: ");
    }

    {
        std::map<int, std::string> data = {{1, "a"}, {2, "b"}, {3, "c"}};
        utils::disp(data, "data: ");
    }

    {
        std::cout << "Current time: " << utils::getTimeStampString()
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
    EXPECT_TRUE(utils::createDirectory(folderName));
    EXPECT_TRUE(utils::isDirectory(folderName));
    EXPECT_TRUE(utils::getAbslutePath(folderName) == folderName);
    EXPECT_TRUE(utils::getAbslutePath("./") == utils::getCurrentFolder());
    EXPECT_TRUE(utils::remove(folderName));
}

TEST(FileSystemUtilities, Negative) {
    std::string folderName = "/usr/test";
    ASSERT_ANY_THROW(utils::createDirectory(folderName));
    EXPECT_FALSE(utils::remove(folderName));
    EXPECT_FALSE(utils::isRegularFile(folderName));
    EXPECT_FALSE(utils::isDirectory("tUnitTests.cpp"));
}

TEST(TemporaryDirectory, Positive) {
    utils::TemporaryDirectory tmpDir;
    std::cout << tmpDir.getPath() << std::endl;
    EXPECT_TRUE(boost::filesystem::exists(tmpDir.getPath()));
}

TEST(ExporeFolderRootLevel, Positive) {
    {
        utils::TemporaryDirectory tmpDir;
        auto results = utils::exploreFolderAtRootLevel(tmpDir.getPath(), 0);
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
