#include "gtest/gtest.h"
#include <array>
#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <vector>

#include "sbutils/DataStructures.hpp"
#include "sbutils/FileSearch.hpp"
#include "sbutils/FileUtils.hpp"
#include "sbutils/Print.hpp"
#include "sbutils/TemporaryDirectory.hpp"
#include "sbutils/Timer.hpp"

#include "TestData.hpp"

using path = boost::filesystem::path;

namespace {
  using Container = std::vector<utils::FileInfo>;

  std::tuple<Container, Container, Container> newdiff(Container &first, Container &second) {
    using value_type  = Container::value_type;
    Container newFiles, modifiedFiles, deletedFiles;
    std::unordered_set<value_type> dict(second.begin(), second.end());

    auto deletedFunc = [&first, &dict, &deletedFiles](auto && item) {
      auto it = dict.find(item);
      if (it == dict.end()) {
        // This file has been deleted or removed.
        deletedFiles.emplace_back(item);
      } else {
        dict.erase(it);
      }
    };

    std::for_each(first.begin(), first.end(), deletedFunc);
    
    return std::make_tuple(std::move(newFiles), std::move(modifiedFiles), std::move(deletedFiles));
  }
}

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

TEST(DataStructure, Positive) {
    // using IArchive = cereal::JSONInputArchive;
    using OArchive = cereal::JSONOutputArchive;
    using value_type = utils::FileInfo;

    utils::TemporaryDirectory tmpDir;
    TestData data(tmpDir.getPath());
        
    auto const aPath = tmpDir.getPath() / path("data/data.mat");

    {
        value_type aFile;

        std::stringstream output;
        {
            OArchive oar(output);
            oar(cereal::make_nvp("Empty object", aFile));
        }
        fmt::print("{}\n", output.str());
    }

    {
        namespace fs = boost::filesystem;
        boost::system::error_code errcode;
        auto const status = fs::status(aPath);
        value_type aFile(status.permissions(),
                         boost::filesystem::file_size(aPath), aPath.string(),
                         aPath.stem().string(), aPath.extension().string(),
                         fs::last_write_time(aPath, errcode));
        std::stringstream output;
        {
            OArchive oar(output);
            oar(cereal::make_nvp(aPath.string(), aFile));
        }
        fmt::print("{}\n", output.str());

        std::unordered_map<std::string, value_type> map;

        std::vector<value_type> v{aFile, aFile};
        std::unordered_set<value_type> dict(v.begin(), v.end());
        
        dict.reserve(4);
        dict.emplace(aFile);
        dict.emplace(aFile);
        
        EXPECT_TRUE(dict.size() == 1);
        EXPECT_TRUE(dict.find(aFile) != dict.end());
        
        map.emplace(std::make_pair("aKey", aFile));
        map["foo"] = aFile;
        map["boo"] = aFile;
        EXPECT_TRUE(map.size() == 3);

        auto results = newdiff(v, v);
    }

    {
        utils::RootFolder aFolder;
        aFolder.update(tmpDir.getPath());
        
        fmt::print("Current path: {}\n", aFolder.Path.string());
        
        auto printObj = [](auto &item){fmt::print("{}\n", item.string());};

        fmt::print("Files:\n");
        std::for_each(aFolder.Files.cbegin(), aFolder.Files.cend(), printObj);

        fmt::print("Folders:\n");
        std::for_each(aFolder.Folders.cbegin(), aFolder.Folders.cend(), printObj);

        EXPECT_TRUE(aFolder.Files.size() == 1);
        EXPECT_TRUE(aFolder.Folders.size() == 7);
    }
}

TEST(DFS, Positive) {
    using path = boost::filesystem::path;
    using OArchive = cereal::JSONOutputArchive;
    
    utils::TemporaryDirectory tmpDir;
    TestData data(tmpDir.getPath());
    std::vector<path> folders{tmpDir.getPath()};
    using FileVisitor =
        utils::filesystem::Visitor<decltype(folders),
                                   utils::filesystem::NormalPolicy>;
    utils::ElapsedTime<utils::MILLISECOND> timer("Total time: ");
    std::stringstream output;
    FileVisitor visitor;
    utils::filesystem::dfs_file_search(std::move(folders), visitor);
    // visitor.print<OArchive>();

    // Get the folder hierarchy
    auto results = visitor.getFolderHierarchy<int>();                                   
    {
        OArchive oar(output);
        oar(cereal::make_nvp("Folder hierarchy", results));
    }

    fmt::print("{}\n", output.str());

    EXPECT_EQ(results.Graph.numberOfVertexes(), static_cast<size_t>(6));
    EXPECT_TRUE(results.Graph.isDirected());
}
