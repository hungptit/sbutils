// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "boost/filesystem.hpp"
#include "gtest/gtest.h"
#include <array>
#include <deque>
#include <string>
#include <vector>

#include "sbutils/BasePathVisitor.hpp"
#include "sbutils/SimplePathVisitor.hpp"
#include "sbutils/PathFilter.hpp"
#include "sbutils/PathSearchAlgorithms.hpp"
#include "sbutils/PathVisitor.hpp"
#include "sbutils/SimplePathVisitor.hpp"
#include "sbutils/PathSearchVisitor.hpp"
#include "sbutils/FileDBVisitor.hpp"
#include "sbutils/TemporaryDirectory.hpp"

#include "TestData.hpp"

#define CATCH_CONFIG_MAIN
#include "catch/catch.hpp"

using String = std::string;

template <typename Visitor> size_t dfs(const boost::filesystem::path &root) {
    using path = typename Visitor::path;
    using PathContainer = typename Visitor::path_container;
    Visitor visitor;

    PathContainer stack{root};
    sbutils::dfs_file_search(stack, visitor);
    return visitor.Paths.size();
}

template <typename Visitor> size_t bfs(const boost::filesystem::path &root) {
    using path = typename Visitor::path;
    using PathContainer = typename Visitor::path_container;
    Visitor visitor;

    PathContainer queue{root};
    sbutils::bfs_file_search(queue, visitor);
    return visitor.Paths.size();
}

TEST_CASE("Policies", "All") {
	SECTION("NullPolicy") {
        sbutils::NullPolicy policy;

        SECTION("We do not care about stem") {
            REQUIRE(policy.isValidStem(""));
            REQUIRE(policy.isValidStem("Foo"));
        }

        SECTION("Check isValidExt") {
            std::vector<String> validExts = {"", ".cpp", ".pm", ".t"};
            std::vector<String> invalidExts = {};
            std::for_each(validExts.cbegin(), validExts.cend(),
                          [&policy](auto const item) { REQUIRE(policy.isValidExt(item)); });
            std::for_each(invalidExts.cbegin(), invalidExts.cend(),
                          [&policy](auto const item) { REQUIRE(!policy.isValidExt(item)); });
        }
    }
	
    SECTION("PerlPolicy") {
        sbutils::PerlPolicy policy;

        SECTION("We do not care about stem") {
            REQUIRE(policy.isValidStem(""));
            REQUIRE(policy.isValidStem("Foo"));
        }

        SECTION("Check isValidExt") {
            std::vector<String> validExts = {".pl", ".pm", ".t"};
            std::vector<String> invalidExts = {".txt", "", ".it"};
            std::for_each(validExts.cbegin(), validExts.cend(),
                          [&policy](auto const item) { REQUIRE(policy.isValidExt(item)); });
            std::for_each(invalidExts.cbegin(), invalidExts.cend(),
                          [&policy](auto const item) { REQUIRE(!policy.isValidExt(item)); });
        }
    }

	SECTION("CppPolicy") {
        sbutils::CppPolicy policy;

        SECTION("We do not care about stem") {
            REQUIRE(policy.isValidStem(""));
            REQUIRE(policy.isValidStem("Foo"));
        }

        SECTION("Check isValidExt") {
            std::vector<String> validExts = {".cpp", ".hpp", ".h"};
            std::vector<String> invalidExts = {".txt", "", ".pl", ".cp"};
            std::for_each(validExts.cbegin(), validExts.cend(),
                          [&policy](auto const item) { REQUIRE(policy.isValidExt(item)); });
            std::for_each(invalidExts.cbegin(), invalidExts.cend(),
                          [&policy](auto const item) { REQUIRE(!policy.isValidExt(item)); });
        }
    }

	SECTION("FolderPolicy") {
        sbutils::FolderPolicy policy;

        SECTION("Check isValidStem") {
			std::vector<String> validStems = {"", "Foo", "Boo"};
            std::vector<String> invalidStems = {"CMakeFiles", "CMakeTmp"};
            std::for_each(validStems.cbegin(), validStems.cend(),
                          [&policy](auto const item) { REQUIRE(policy.isValidStem(item)); });
            std::for_each(invalidStems.cbegin(), invalidStems.cend(),
                          [&policy](auto const item) { REQUIRE(!policy.isValidStem(item)); });
        }

        SECTION("Check isValidExt") {
            std::vector<String> validExts = {".cpp", ".hpp", ".h"};
            std::vector<String> invalidExts = {".git", ".cache"};
            std::for_each(validExts.cbegin(), validExts.cend(),
                          [&policy](auto const item) { REQUIRE(policy.isValidExt(item)); });
            std::for_each(invalidExts.cbegin(), invalidExts.cend(),
                          [&policy](auto const item) { REQUIRE(!policy.isValidExt(item)); });
        }
    }
}

TEST_CASE("Visitors", "") {
    using path = boost::filesystem::path;
    using String = std::string;
    sbutils::TemporaryDirectory tmpDir;
    TestData testData(tmpDir.getPath(), true);

    SECTION("NullPolicy") {
        using String = std::string;
        using path = boost::filesystem::path;
        using PathContainer = std::vector<path>;
        using FileFilter = sbutils::NullPolicy;
		using DirFilter = sbutils::NullPolicy;
		using BaseVisitor = typename sbutils::BasePathVisitor<String, PathContainer>;
		sbutils::PathSearchVisitor<BaseVisitor, FileFilter, DirFilter> visitor(DirFilter(), FileFilter(), "");
        PathContainer stack;
        visitor.visit(tmpDir.getPath(), stack);
        visitor.visit(tmpDir.getPath(), stack);

		fmt::print("{}\n", visitor.Paths[0]);
        REQUIRE(visitor.Paths.size() == 1); // There is only one valid file at root level.
        REQUIRE(stack.size() == 7);			// There are 7 folders in the stack.
    }

    SECTION("NormalPolicy") {
        using PathContainer = std::vector<path>;
		using FileFilter = sbutils::NullPolicy;
		using DirFilter = sbutils::NullPolicy;
        sbutils::FileDBVisitor<String, PathContainer, FileFilter, DirFilter> visitor;
        PathContainer stack;
        visitor.visit(tmpDir.getPath(), stack);
        visitor.visit(tmpDir.getPath(), stack);
        
        REQUIRE(visitor.Paths.size() == 8); // Will skip .git and .subversion
        REQUIRE(stack.size() == 7);
    }

    SECTION("File filter") {
        using PathContainer = std::vector<path>;
        using CustomFilter = sbutils::NullPolicy;
    }
}

TEST_CASE("File search algorithms using NullPolicy", "dfs/bfs") {
    using path = boost::filesystem::path;
    using String = std::string;
    sbutils::TemporaryDirectory tmpDir;
    TestData testData(tmpDir.getPath());

    SECTION("dfs search algorithm") {
        using String = std::string;
        using PathContainer = std::vector<path>;
		using BaseVisitor = typename sbutils::BasePathVisitor<String, PathContainer>;
		
        SECTION("SimpleFileVisitor") {
            SECTION("NullPolicy") {
                using CustomFilter = sbutils::NullPolicy;
				
                using Visitor =
                    typename sbutils::SimplePathVisitor<BaseVisitor, CustomFilter>;
                REQUIRE(dfs<Visitor>(tmpDir.getPath()) == 13);
            }

            SECTION("NullPolicy") {
                using CustomFilter = sbutils::NullPolicy;
                using Visitor =
                    typename sbutils::SimplePathVisitor<BaseVisitor, CustomFilter>;
                REQUIRE(dfs<Visitor>(tmpDir.getPath()) == 13);
            }
        }

        SECTION("FileDBVisitor") {
            SECTION("NullPolicy") {
                using FileFilter = sbutils::NullPolicy;
				using DirFilter = sbutils::NullPolicy;
                using Visitor =
                    typename sbutils::FileDBVisitor<String, PathContainer, FileFilter, DirFilter>;
                REQUIRE(dfs<Visitor>(tmpDir.getPath()) == 20);
            }

            SECTION("NullPolicy") {
                using CustomFilter = sbutils::NullPolicy;
                using Visitor =
                    typename sbutils::FileDBVisitor<String, PathContainer, CustomFilter, CustomFilter>;
                REQUIRE(dfs<Visitor>(tmpDir.getPath()) == 20);
            }
        }
    }

    SECTION("bfs search algorithm") {
        using PathContainer = std::deque<path>;
        using String = std::string;
		using BaseVisitor = typename sbutils::BasePathVisitor<String, PathContainer>;
		
        SECTION("Simple visitor") {
            SECTION("NullPolicy") {
                using CustomFilter = sbutils::NullPolicy;
                using Visitor =
                    typename sbutils::SimplePathVisitor<BaseVisitor, CustomFilter>;
                REQUIRE(bfs<Visitor>(tmpDir.getPath()) == 13);
            }

            SECTION("NullPolicy") {
                using CustomFilter = sbutils::NullPolicy;
                using Visitor =
                    typename sbutils::SimplePathVisitor<BaseVisitor, CustomFilter>;
                REQUIRE(bfs<Visitor>(tmpDir.getPath()) == 13);
            }
        }

        SECTION("FileDBVisitor") {
            SECTION("NullPolicy") {
                using CustomFilter = sbutils::NullPolicy;
                using Visitor =
                    typename sbutils::SimplePathVisitor<BaseVisitor, CustomFilter>;
                REQUIRE(bfs<Visitor>(tmpDir.getPath()) == 13);
            }

            SECTION("NullPolicy") {
                using CustomFilter = sbutils::NullPolicy;
                using Visitor =
                    typename sbutils::SimplePathVisitor<BaseVisitor, CustomFilter>;
                REQUIRE(bfs<Visitor>(tmpDir.getPath()) == 13);
            }
        }
    }
}
