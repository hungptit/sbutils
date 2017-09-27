// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "boost/filesystem.hpp"
#include "gtest/gtest.h"
#include <array>
#include <deque>
#include <string>
#include <vector>

#include "sbutils/FileDB.hpp"
#include "sbutils/PathSearchAlgorithms.hpp"
#include "sbutils/PathFilter.hpp"
#include "sbutils/TemporaryDirectory.hpp"

#include "TestData.hpp"

#define CATCH_CONFIG_MAIN
#include "catch/catch.hpp"

template <typename Visitor>
size_t dfs(const boost::filesystem::path &root) {
    using path = typename Visitor::path;
    using String = std::string;
    using PathContainer = typename Visitor::path_container;
    Visitor visitor;

    PathContainer stack{root};
    sbutils::dfs_file_search(stack, visitor);
    return visitor.Paths.size();
}

template <typename Visitor>
size_t bfs(const boost::filesystem::path &root) {
    using path = typename Visitor::path;
    using String = std::string;
    using PathContainer = typename Visitor::path_container;
    Visitor visitor;

    PathContainer queue{root};
    sbutils::bfs_file_search(queue, visitor);
    return visitor.Paths.size();
}

TEST_CASE("File search algorithms", "dfs/bfs") {
    using path = boost::filesystem::path;
    using String = std::string;
    sbutils::TemporaryDirectory tmpDir;
    TestData testData(tmpDir.getPath());

    SECTION("NullPolicy") {
        using PathContainer = std::vector<path>;
        using CustomFilter = sbutils::NullPolicy;
        sbutils::FileDBVisitor<String, PathContainer, CustomFilter> visitor;
        PathContainer stack;
        visitor.visit(tmpDir.getPath(), stack);
        visitor.visit(tmpDir.getPath(), stack);

        visitor.print();

        REQUIRE(visitor.Paths.size() == 8);
        REQUIRE(stack.size() == 7);
    }

    SECTION("NormalPolicy") {
        using PathContainer = std::vector<path>;
        using CustomFilter = sbutils::NormalPolicy;
        sbutils::FileDBVisitor<String, PathContainer, CustomFilter> visitor;
        PathContainer stack;
        visitor.visit(tmpDir.getPath(), stack);
        visitor.visit(tmpDir.getPath(), stack);

        visitor.print();

        REQUIRE(visitor.Paths.size() == 6); // Will skip .git and .subversion
        REQUIRE(stack.size() == 5);
    }

    SECTION("dfs search algorithm") {
		using String = std::string;
		using PathContainer = std::vector<path>;

		SECTION ("SimpleFileVisitor") {
			SECTION("NullPolicy") {
				using CustomFilter = sbutils::NullPolicy;
				using Visitor = typename sbutils::SimpleFileVisitor<String, PathContainer, CustomFilter>;
				REQUIRE(dfs<Visitor>(tmpDir.getPath()) == 20);
			}

			SECTION("NormalPolicy") {
				using CustomFilter = sbutils::NormalPolicy;
				using Visitor = typename sbutils::SimpleFileVisitor<String, PathContainer, CustomFilter>;
				REQUIRE(dfs<Visitor>(tmpDir.getPath()) == 17);
			}
		}

		SECTION ("FileDBVisitor") {
			SECTION("NullPolicy") {
				using CustomFilter = sbutils::NullPolicy;
				using Visitor = typename sbutils::FileDBVisitor<String, PathContainer, CustomFilter>;
				REQUIRE(dfs<Visitor>(tmpDir.getPath()) == 20);
			}

			SECTION("NormalPolicy") {
				using CustomFilter = sbutils::NormalPolicy;
				using Visitor = typename sbutils::FileDBVisitor<String, PathContainer, CustomFilter>;
				REQUIRE(dfs<Visitor>(tmpDir.getPath()) == 17);
			}
		}
    }

    SECTION("bfs search algorithm") {
		using PathContainer = std::deque<path>;
		using String = std::string;

		SECTION("Simple visitor") {
			SECTION("NullPolicy") {
				using CustomFilter = sbutils::NullPolicy;
				using Visitor = typename sbutils::SimpleFileVisitor<String, PathContainer, CustomFilter>;
				REQUIRE(bfs<Visitor>(tmpDir.getPath()) == 20);
			}

			SECTION("NormalPolicy") {
				using CustomFilter = sbutils::NormalPolicy;
				using Visitor = typename sbutils::SimpleFileVisitor<String, PathContainer, CustomFilter>;
				REQUIRE(bfs<Visitor>(tmpDir.getPath()) == 17);
			}
		}
		
		SECTION("FileDBVisitor") {
			SECTION("NullPolicy") {
				using CustomFilter = sbutils::NullPolicy;
				using Visitor = typename sbutils::SimpleFileVisitor<String, PathContainer, CustomFilter>;
				REQUIRE(bfs<Visitor>(tmpDir.getPath()) == 20);
			}

			SECTION("NormalPolicy") {
				using CustomFilter = sbutils::NormalPolicy;
				using Visitor = typename sbutils::SimpleFileVisitor<String, PathContainer, CustomFilter>;
				REQUIRE(bfs<Visitor>(tmpDir.getPath()) == 17);
			}
		}
    }
}
