#include "utils/Basic.hpp"
#include "utils/Resources.hpp"
#include "utils/FileSystemUtilities.hpp"
#include "utils/FileSearchStrategy.hpp"
#include "utils/FileFinder.hpp"
#include "utils/TimeUtilities.hpp"

#include "gtest/gtest.h"
#include <string>
#include <map>
#include <vector>
#include <array>
#include <tuple>

TEST(Display_Functions, Positive) {
    {
        std::vector<int> data = {1, 2, 3, 4, 5, 6};
        Tools::disp(data, "data: ");
    }

    {
        std::map<int, std::string> data = {{1, "a"}, {2, "b"}, {3, "c"}};
        Tools::disp_pair(data, "data: ");
    }

    {
        std::cout << "Current time: " << Tools::getTimeStampString()
                  << std::endl;
    }
}

TEST(GetEnv, Positive) {
    {
        std::string results = Tools::getenv("PATH");
        EXPECT_TRUE(!results.empty());
    }

    {
        std::string results = Tools::getenv("PATH1");
        EXPECT_TRUE(results.empty());
    }
}

TEST(FileSystemUtilities, Positive) {
    std::string folderName = Tools::TemporaryFolder<std::string>::value +
        Tools::FileSeparator<std::string>::value + "test";
    EXPECT_TRUE(Tools::createDirectory(folderName));
    EXPECT_TRUE(Tools::isDirectory(folderName));
    EXPECT_TRUE(Tools::getAbslutePath(folderName) == folderName);
    EXPECT_TRUE(Tools::getAbslutePath("./") == Tools::getCurrentFolder());
    EXPECT_TRUE(Tools::remove(folderName));
}

TEST(FileSystemUtilities, Negative) {
    std::string folderName = "/usr/test";
    ASSERT_ANY_THROW(Tools::createDirectory(folderName));
    EXPECT_FALSE(Tools::remove(folderName));
    EXPECT_FALSE(Tools::isRegularFile(folderName));
    EXPECT_FALSE(Tools::isDirectory("tUnitTests.cpp"));
}


TEST(TemporaryDirectory, Positive) {
    Tools::TemporaryDirectory tmpDir;
    std::cout << tmpDir.getPath() << std::endl;
    EXPECT_TRUE(boost::filesystem::exists(tmpDir.getPath()));
}
