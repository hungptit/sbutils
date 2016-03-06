#include "utils/Utils.hpp"
#include "utils/SparseGraph.hpp"
#include <iostream>
#include <tuple>
#include <vector>
#include "gtest/gtest.h"

template <typename Index>
std::vector<std::tuple<Index, Index>> createTestData() {
  std::vector<std::tuple<Index, Index>> edges{
    std::make_tuple(0, 1), std::make_tuple(0, 2), std::make_tuple(2, 2),
      std::make_tuple(0, 3), std::make_tuple(1, 4), std::make_tuple(2, 5),
      std::make_tuple(5, 7), std::make_tuple(3, 5), std::make_tuple(3, 6),
      std::make_tuple(4, 7), std::make_tuple(6, 4), std::make_tuple(6, 1),
      std::make_tuple(7, 6)};
  std::sort(edges.begin(), edges.end());
  return edges;
}

template <typename Container> auto vertex_num(const Container &data) {
  if (data.empty())
    return 0;
  auto begin = data.begin();
  auto maxVal = std::get<0>(*begin);
  for (auto it = ++begin; it != data.end(); ++it) {
    if (std::get<0>(*it) > maxVal) {
      maxVal = std::get<0>(*it);
    }
  }
  return maxVal + 1;
}

TEST(TestSparseGraph, Positive) {
  auto edges = createTestData<int>();
  std::cout << "==== Edge information ====\n";
  utils::print(edges);
  utils::SparseGraph<int> g(edges, vertex_num(edges), true);

  {
    auto vertexes = g.getVertexes();
    decltype(vertexes) expectedVertexes{ 0, 3, 4, 6, 8, 9, 10, 12, 13 };
    EXPECT_EQ(expectedVertexes, vertexes);

    auto edges = g.getEdges();
    decltype(edges) expectedEdges { 1, 2, 3, 4, 2, 5, 5, 6, 7, 7, 1, 4, 6 };
    EXPECT_EQ(expectedEdges, edges);
  }
  
  {
    auto e = g.edges(0);
    std::vector<int> results(std::get<0>(e), std::get<1>(e));
    decltype(results) expectedResults{1, 2, 3};
    EXPECT_EQ(results, expectedResults);
  }

    {
    auto e = g.edges(6);
    std::vector<int> results(std::get<0>(e), std::get<1>(e));
    decltype(results) expectedResults{1, 4};
    EXPECT_EQ(expectedResults, results);
  }

        {
    auto e = g.edges(7);
    std::vector<int> results(std::get<0>(e), std::get<1>(e));
    decltype(results) expectedResults {6};
    EXPECT_EQ(expectedResults, results);
  }

  // Check that we can generate a dot graph.
  {
    utils::graph_info(g);
    std::vector<std::string> v{"-0-", "-1-", "-2-", "-3-",
        "-4-", "-5-", "-6-", "-7-"};
    std::string dotFile("test.dot");
    utils::gendot(g, v, dotFile);
    // utils::viewdot(dotFile);
  }
}

TEST(DFS, Positive) {
  auto edges = createTestData<int>();
  std::cout << "==== Edge information ====\n";
  utils::print(edges);
  utils::SparseGraph<int> g(edges, vertex_num(edges), true);

  utils::DFS<decltype(g)> alg;
  fmt::print("Visited vertexes\n");
  {
    auto results = alg.dfs(g, 0);
    decltype(results) expectedResults { 0, 3, 6, 4, 7, 1, 5, 2 };
    utils::print(results);
    EXPECT_EQ(results, expectedResults);
  }
}

TEST(BFS, Positive) {
  auto edges = createTestData<int>();
  std::cout << "==== Edge information ====\n";
  utils::print(edges);
  utils::SparseGraph<int> g(edges, vertex_num(edges), true);

  utils::BFS<decltype(g)> alg;
  fmt::print("Visited vertexes\n");
  {
    auto results = alg.bfs(g, 0);
    decltype(results) expectedResults { 0, 1, 2, 3, 4, 5, 6, 7 };
    utils::print(results);
    EXPECT_EQ(results, expectedResults);
  }
}
