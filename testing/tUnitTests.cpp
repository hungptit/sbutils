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
#include <unordered_map>
#include <tuple>

TEST(Display_Functions, Positive) 
{
    {
        std::vector<int> data = {1,2,3,4,5,6};
        Tools::disp(data, "data: ");
        Tools::
    }

    {
        std::map<int, std::string> data = {{1, "a"},{2, "b"},{3, "c"}};
        Tools::disp_pair(data, "data: ");
    }

    
    {
        std::cout << "Current time: " << Tools::getTimeStampString() << std::endl;
    }
}


TEST(GetEnv, Positive) 
{
    {
        std::string results = Tools::getEnvironmentVariableValue("PATH");
        EXPECT_TRUE(!results.empty());
    }

    {
        std::string results = Tools::getEnvironmentVariableValue("PATH1");
        EXPECT_TRUE(results.empty());
    }
}


TEST(FileSystemUtilities, Positive) 
{
    std::string folderName = Tools::TemporaryFolder<std::string>::value + Tools::FileSeparator<std::string>::value + "test";
    EXPECT_TRUE(Tools::createDirectory(folderName));
    EXPECT_TRUE(Tools::isDirectory(folderName));
    EXPECT_TRUE(Tools::getAbslutePath(folderName) == folderName);
    EXPECT_TRUE(Tools::getAbslutePath("./") == Tools::getCurrentFolder());
    EXPECT_TRUE(Tools::remove(folderName));
}


TEST(FileSystemUtilities, Negative) 
{
    std::string folderName = "/usr/test";
    ASSERT_ANY_THROW(Tools::createDirectory(folderName));
    EXPECT_FALSE(Tools::remove(folderName));
    EXPECT_FALSE(Tools::isRegularFile(folderName));
    EXPECT_FALSE(Tools::isDirectory("tUnitTests.cpp"));
}


TEST(FileSearchStrategy, Positive)
{
    {
        std::string fileName = "tUnitTests.cpp";
        boost::filesystem::path afile(fileName);
        boost::filesystem::file_status fs = boost::filesystem::status(afile);
        EXPECT_TRUE(Tools::FileSearchStrategy<Tools::FileTypeTag::All>::apply(fs));
        EXPECT_TRUE(Tools::FileSearchStrategy<Tools::FileTypeTag::Edited>::apply(fs));
        EXPECT_FALSE(Tools::FileSearchStrategy<Tools::FileTypeTag::OwnerReadOnly>::apply(fs));
    }


    {
        std::string fileName = "data/data.txt";
        boost::filesystem::path afile(fileName);
        boost::filesystem::file_status fs = boost::filesystem::status(afile);
        EXPECT_FALSE(Tools::FileSearchStrategy<Tools::FileTypeTag::Edited>::apply(fs));
        EXPECT_TRUE(Tools::FileSearchStrategy<Tools::FileTypeTag::OwnerReadOnly>::apply(fs));
    }

    // File extension strategy
    {
        typedef std::unordered_map<std::string, bool> Map;
        Map supportedExts = {{".txt", true}, {".dat", true}, {".mat", true}};
        EXPECT_TRUE((Tools::FileExtension<Tools::FileExtensionTag::All, Map>::apply(".foo", supportedExts)));
        EXPECT_TRUE((Tools::FileExtension<Tools::FileExtensionTag::Specific, Map>::apply(".txt", supportedExts)));
        EXPECT_FALSE((Tools::FileExtension<Tools::FileExtensionTag::Specific, Map>::apply(".foo", supportedExts)));
    }


    // File name strategy
    {
        std::string fileName = "/this/is/a/test/file.foo";
        const boost::regex expression1(".*this.*/file.foo");
        const boost::regex expression2("file1");
        EXPECT_TRUE(Tools::FileName<Tools::FileNameTag::All>::apply(fileName, expression1));
        EXPECT_TRUE(Tools::FileName<Tools::FileNameTag::Specific>::apply(fileName, expression1));
        EXPECT_FALSE(Tools::FileName<Tools::FileNameTag::Specific>::apply(fileName, expression2));
    }


    {
        typedef std::unordered_map<std::string, bool> Map;
        const Map supportedExts = {{".txt", true}, {".dat", true}, {".mat", true}};
        const boost::regex expressions("dat\\w/(src)*/data.txt");
        
        {
            typedef Tools::FileSearchStrategy<Tools::FileTypeTag::All> FSearchStrategy;
            typedef Tools::FileExtension<Tools::FileExtensionTag::Specific, Map> FExtSearchStrategy;
            typedef Tools::FileName<Tools::FileNameTag::All> FNameSearchStrategy;
            Tools::FileFinder<FSearchStrategy, FExtSearchStrategy, FNameSearchStrategy> fSearch;
            fSearch.search("data", supportedExts, expressions);
            fSearch.print();
            const auto & data = fSearch.getData();
            EXPECT_TRUE(data.size() == 2);

	    // Results order might be changed on different system.
            // EXPECT_TRUE(data[1].first == "data/data.txt");
            // EXPECT_TRUE(data[1].second == 256);
            // EXPECT_TRUE(data[0].first == "data/src/data.txt");
            // EXPECT_TRUE(data[0].second == 420);
        }

        {
            typedef Tools::FileSearchStrategy<Tools::FileTypeTag::All> FSearchStrategy;
            typedef Tools::FileExtension<Tools::FileExtensionTag::Specific, Map> FExtSearchStrategy;
            typedef Tools::FileName<Tools::FileNameTag::Specific> FNameSearchStrategy;
            Tools::FileFinder<FSearchStrategy, FExtSearchStrategy, FNameSearchStrategy> fSearch;
            fSearch.search("data", supportedExts, expressions);
            fSearch.print();
            const auto & data = fSearch.getData();
            EXPECT_TRUE(data.size() == 1);
            EXPECT_TRUE(data[0].first == "data/src/data.txt");
            EXPECT_TRUE(data[0].second == 420);
        }
    }
}
