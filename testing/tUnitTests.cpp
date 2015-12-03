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
#include "utils/LevelDBIO.hpp"
#include "utils/TemporaryDirectory.hpp"
#include "utils/Timer.hpp"
#include "utils/Utils.hpp"

#include "SetupTest.hpp"

using path = boost::filesystem::path;

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
    EXPECT_TRUE(dfs_finder.getData().size() == 6);
    EXPECT_TRUE(dfs_finder.getData().size() == bfs_finder.getData().size());

    // Test filter methods
    std::vector<std::string> stems = {"foo"};
    std::vector<std::string> exts = {".txt"};
    auto results = dfs_finder.filter(stems, exts);
    Utils::print(results);
    EXPECT_EQ(results.size(), static_cast<size_t>(2));
}

TEST(FileDatabase, Positive) {
    SetupTestDirectory setup;
    auto aPath = setup.getCurrentPath();
    Utils::FileSearchBase<Utils::DFSFileSearchBase> dfs_finder;
    std::cout << "Number of files: " << dfs_finder.search(aPath) << std::endl;

    // Serialized file information to string
    std::ostringstream os;
    auto data = dfs_finder.getData();
    Utils::save<Utils::OArchive, decltype(data)>(data, os);
    const auto value = os.str();

    // Write data to the database.
    path dataFile = aPath / path(Utils::FileDatabaseInfo::Database);
    auto key = dataFile.string();
    {
        Utils::Writer writer(dataFile.string());
        writer.write(key, value);
    }

    // Now read the data back and compare with the original data.
    {
        Utils::Timer timer;
        Utils::Reader reader(dataFile.string());
        std::istringstream is(reader.read(key));
        std::vector<Utils::FileInfo> rdata;
        Utils::load<Utils::IArchive, decltype(rdata)>(rdata, is);
        std::cout << "Deserialize time: "
                  << timer.toc() / timer.ticksPerSecond() << " seconds"
                  << std::endl;
        Utils::print(rdata);
        EXPECT_TRUE(rdata.size() == 6);
        EXPECT_EQ(data, rdata);
    }
}
