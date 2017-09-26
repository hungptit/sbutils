// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "boost/filesystem.hpp"
#include "gtest/gtest.h"
#include <array>
#include <fstream>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>
#include <deque>

#include "sbutils/FileSearch.hpp"
#include "sbutils/Print.hpp"
#include "sbutils/TemporaryDirectory.hpp"
#include "sbutils/FileDB.hpp"
#include "sbutils/PathFilter.hpp"

#include "TestData.hpp"

#define CATCH_CONFIG_MAIN
#include "catch/catch.hpp"

TEST_CASE("Filter", "") {
	// DonothingPolicy
    {
		using path = boost::filesystem::path;
		using String = std::string;
		using PathContainer = std::vector<path>;
		using CustomFilter = sbutils::NullPolicy;
		filesystem::FileDBVisitor<String, PathContainer, CustomFilter> visitor;

		PathContainer stack;
		visitor.visit("./", stack);
		visitor.visit("CMakeFiles", stack);
		visitor.visit("CMakeFiles", stack);
		visitor.print();
    }

	// NormalPolicy
    {
		using path = boost::filesystem::path;
		using String = std::string;
		using PathContainer = std::vector<path>;
		using CustomFilter = sbutils::NormalPolicy;
		filesystem::FileDBVisitor<String, PathContainer, CustomFilter> visitor;

		PathContainer stack;
		visitor.visit("./", stack);
		visitor.visit("CMakeFiles", stack);
		visitor.visit("CMakeFiles", stack);
		visitor.print();
    }
}

template<typename Filter>
size_t dfs(const boost::filesystem::path &root, const bool verbose = false) {
	using path = boost::filesystem::path;
	using String = std::string;
	using PathContainer = std::vector<path>;
	filesystem::FileDBVisitor<String, PathContainer, Filter> visitor;

	PathContainer stack {root};
	sbutils::dfs_file_search(stack, visitor);
	if (verbose) visitor.print();
	return visitor.Paths.size();
}

template<typename Filter>
size_t bfs(const boost::filesystem::path &root, const bool verbose = false) {
	using path = boost::filesystem::path;
	using String = std::string;
	using PathContainer = std::deque<path>;
	filesystem::FileDBVisitor<String, PathContainer, Filter> visitor;

	PathContainer queue {root};
	sbutils::bfs_file_search(queue, visitor);
	if (verbose) visitor.print();
	return visitor.Paths.size();
}

TEST_CASE("File search algorithms", "dfs/bfs") {
	sbutils::TemporaryDirectory tmpDir;
    TestData testData(tmpDir.getPath());
	SECTION("dfs search algorithm") {
		REQUIRE(dfs<sbutils::NullPolicy>(tmpDir.getPath()) == 20);
		REQUIRE(dfs<sbutils::NormalPolicy>(tmpDir.getPath()) == 17);
	}
	SECTION("bfs search algorithm") {
		REQUIRE(bfs<sbutils::NullPolicy>(tmpDir.getPath()) == 20);
		REQUIRE(bfs<sbutils::NormalPolicy>(tmpDir.getPath()) == 17);
	}
}
