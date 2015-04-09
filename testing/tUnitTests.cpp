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

TEST(FileSearchStrategy, Positive) {
    {
        std::string fileName = "tUnitTests.cpp";
        boost::filesystem::path afile(fileName);
        boost::filesystem::file_status fs = boost::filesystem::status(afile);
        EXPECT_TRUE(
            Tools::FileSearchStrategy<Tools::FileTypeTag::All>::apply(fs));
        EXPECT_TRUE(
            Tools::FileSearchStrategy<Tools::FileTypeTag::Edited>::apply(fs));
        EXPECT_FALSE(
            Tools::FileSearchStrategy<Tools::FileTypeTag::OwnerReadOnly>::apply(
                fs));
    }

    {
        std::string fileName = "data/data.txt";
        boost::filesystem::path afile(fileName);
        boost::filesystem::file_status fs = boost::filesystem::status(afile);
        EXPECT_FALSE(
            Tools::FileSearchStrategy<Tools::FileTypeTag::Edited>::apply(fs));
        EXPECT_TRUE(
            Tools::FileSearchStrategy<Tools::FileTypeTag::OwnerReadOnly>::apply(
                fs));
    }

    // File extension strategy
    {
        typedef std::vector<std::string> Map;
        Map supportedExts = {".txt", ".dat", ".mat"};
        std::sort(supportedExts.begin(), supportedExts.end());

        EXPECT_TRUE(
            (Tools::FileExtension<Tools::FileExtensionTag::All, Map>::apply(
                ".foo", supportedExts)));
        EXPECT_TRUE((
                        Tools::FileExtension<Tools::FileExtensionTag::Specific,
                        Map>::apply(
                            ".txt", supportedExts)));
        EXPECT_FALSE((
                         Tools::FileExtension<Tools::FileExtensionTag::Specific,
                         Map>::apply(
                             ".foo", supportedExts)));
    }

    // File name strategy
    {
        std::string fileName = "/this/is/a/test/file.foo";
        const boost::regex expression1(".*this.*/file.foo");
        const boost::regex expression2("file1");
        EXPECT_TRUE(Tools::FileName<Tools::FileNameTag::All>::apply(
                        fileName, expression1));
        EXPECT_TRUE(Tools::FileName<Tools::FileNameTag::Specific>::apply(
                        fileName, expression1));
        EXPECT_FALSE(Tools::FileName<Tools::FileNameTag::Specific>::apply(
                         fileName, expression2));
    }
}


TEST(FileSearchDefault, Positive) {
    Tools::FileFinder fSearch;

    fSearch.search("data");
    fSearch.print();
    const auto &data = fSearch.getData();
    EXPECT_TRUE(data.size() == 6);

    // Results order might be changed for different system.
    EXPECT_TRUE(data[0].first == "data/data.txt");
    EXPECT_TRUE(data[1].first == "data/src");
}


TEST(FileSearchOneTemplateArg, Positive) {    
    typedef Tools::FileSearchStrategy<Tools::FileTypeTag::All>
        FSearchStrategy;
            
    Tools::FileFinder fSearch;
           
    fSearch.search<FSearchStrategy>("data");
    fSearch.print();
    const auto &data = fSearch.getData();
    EXPECT_TRUE(data.size() == 5);

    // Results order might be changed for different system.
    EXPECT_TRUE(data[0].first == "data/data.txt");
    EXPECT_TRUE(data[1].first == "data/src/data.txt");
}

TEST(FileSearchTwoTemplateArgs, Positive) {
    
}

TEST(FileSearchThreeTemplateArgs, Positive) {
    // Find files based on given constraints
    {
        const boost::regex expressions("dat\\w/(src)*/data.txt");

        {
            typedef std::vector<std::string> Map;
            Map supportedExts = {".dat", ".txt", ".mat"};
            std::sort(supportedExts.begin(), supportedExts.end());

            typedef Tools::FileSearchStrategy<Tools::FileTypeTag::All>
                FSearchStrategy;
            
            typedef Tools::FileExtension<Tools::FileExtensionTag::Specific,
                                         Map>
                FExtSearchStrategy;
            
            typedef Tools::FileName<Tools::FileNameTag::All>
                FNameSearchStrategy;
            
            Tools::FileFinder fSearch;
            
            fSearch.search<FSearchStrategy, FExtSearchStrategy, FNameSearchStrategy>("data", supportedExts, expressions);
            fSearch.print();
            const auto &data = fSearch.getData();
            EXPECT_TRUE(data.size() == 2);

            // Results order might be changed for different system.
            EXPECT_TRUE(data[0].first == "data/data.txt");
            EXPECT_TRUE(data[0].second == 256);
            EXPECT_TRUE(data[1].first == "data/src/data.txt");
            EXPECT_TRUE(data[1].second == 420);
        }

        {
            typedef std::list<std::string> Map;
            const Map supportedExts = {".dat", ".txt", ".mat"};

            typedef Tools::FileSearchStrategy<Tools::FileTypeTag::All>
                FSearchStrategy;
            typedef Tools::FileExtension<Tools::FileExtensionTag::Specific, Map>
                FExtSearchStrategy;            

            typedef Tools::FileName<Tools::FileNameTag::Specific>
                FNameSearchStrategy;

            Tools::FileFinder fSearch;
            fSearch.search<FSearchStrategy, FExtSearchStrategy, FNameSearchStrategy>("data", supportedExts, expressions);
            fSearch.print();
            const auto &data = fSearch.getData();
            EXPECT_TRUE(data.size() == 1);
            EXPECT_TRUE(data[0].first == "data/src/data.txt");
            EXPECT_TRUE(data[0].second == 420);
        }


        {
            typedef std::array<std::string, 3> Map;
            const Map supportedExts = {".dat", ".txt", ".mat"};

            typedef Tools::FileSearchStrategy<Tools::FileTypeTag::All>
                FSearchStrategy;
            typedef Tools::FileExtension<Tools::FileExtensionTag::Specific, Map>
                FExtSearchStrategy;            

            typedef Tools::FileName<Tools::FileNameTag::Specific>
                FNameSearchStrategy;

            Tools::FileFinder fSearch;
            fSearch.search<FSearchStrategy, FExtSearchStrategy, FNameSearchStrategy>("data", supportedExts, expressions);
            fSearch.print();
            const auto &data = fSearch.getData();
            EXPECT_TRUE(data.size() == 1);
            EXPECT_TRUE(data[0].first == "data/src/data.txt");
            EXPECT_TRUE(data[0].second == 420);
        }
    }
}


TEST(TemporaryDirectory, Positive) {
    Tools::TemporaryDirectory tmpDir;
    std::cout << tmpDir.getPath() << std::endl;
    EXPECT_TRUE(boost::filesystem::exists(tmpDir.getPath()));
}
