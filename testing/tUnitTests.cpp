#include "utils/Utils.hpp"
#include "gtest/gtest.h"
#include <string>
#include <map>
#include <vector>
#include <array>
#include <tuple>
#include <iostream>

TEST(Display_Functions, Positive) {
    {
        std::vector<int> data = {1, 2, 3, 4, 5, 6};
        Tools::disp(data, "data: ");
    }

    {
        std::map<int, std::string> data = {{1, "a"}, {2, "b"}, {3, "c"}};
        Tools::disp(data, "data: ");
    }

    {
        std::cout << "Current time: " << Tools::getTimeStampString()
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
