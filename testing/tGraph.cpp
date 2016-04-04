#include "utils/SparseGraph.hpp"
#include "gtest/gtest.h"
#include <iostream>
#include <tuple>
#include <vector>

#include "cppformat/format.h"

#include "utils/Print.hpp"
#include "utils/Serialization.hpp"
#include "utils/SparseGraph.hpp"

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

auto get_vertex_ids() {
    return std::vector<std::string>{"0", "1", "2", "3", "4", "5", "6", "7"};
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
    utils::SparseGraph<int, int> g(edges, vertex_num(edges), true);

    {
        auto vertexes = g.getVertexes();
        decltype(vertexes) expectedVertexes{0, 3, 4, 6, 8, 9, 10, 12, 13};
        EXPECT_EQ(expectedVertexes, vertexes);

        auto edges = g.getEdges();
        decltype(edges) expectedEdges{1, 2, 3, 4, 2, 5, 5, 6, 7, 7, 1, 4, 6};
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
        decltype(results) expectedResults{6};
        EXPECT_EQ(expectedResults, results);
    }

    // Check that we can generate a dot graph.
    {
        fmt::MemoryWriter writer;
        utils::graph::graph_info(g, writer);
        fmt::print("{}\n", writer.str());
        std::vector<std::string> v = get_vertex_ids();
        std::string dotFile("test.dot");
        utils::graph::gendot<decltype(g), fmt::MemoryWriter>(g, v, dotFile);
        // utils::viewdot(dotFile);
    }
}

TEST(DFS, Positive) {
    auto edges = createTestData<int>();
    std::cout << "==== Edge information ====\n";
    utils::print(edges);
    utils::SparseGraph<int, int> g(edges, vertex_num(edges), true);

    fmt::print("Visited vertexes\n");
    {
        using vertex_type = int;
        using Container = std::vector<vertex_type>;
        auto vertexes = g.getVertexes();
        auto edges = g.getEdges();
        utils::graph::NormalVisitor<decltype(g), Container> visitor(
            vertexes.size() - 1);
        utils::graph::dfs(g, visitor, {0});
        auto results = visitor.getResults();
        decltype(results) expectedResults{0, 3, 6, 4, 7, 1, 5, 2};
        utils::print(results);
        EXPECT_EQ(results, expectedResults);
    }

    // Generate a dot graph for a test graph.
    {
        std::stringstream writer;
        utils::graph::graph_info(g, writer);
        fmt::print("{}\n", writer.str());
        std::vector<std::string> v = get_vertex_ids();
        std::string dotFile("test.dot");
        utils::graph::gendot(g, v, dotFile);
        // utils::viewdot(dotFile);
    }
}

TEST(BFS, Positive) {
    auto edges = createTestData<int>();
    std::cout << "==== Edge information ====\n";
    utils::print(edges);
    utils::SparseGraph<int, int> g(edges, vertex_num(edges), true);

    fmt::print("Visited vertexes\n");
    {
        using vertex_type = int;
        using Container = std::deque<vertex_type>;
        auto vertexes = g.getVertexes();
        auto edges = g.getEdges();
        utils::graph::NormalVisitor<decltype(g), Container> visitor(
            vertexes.size() - 1);
        utils::graph::bfs(g, visitor, {0});
        auto results = visitor.getResults();
        decltype(results) expectedResults{0, 1, 2, 3, 4, 5, 6, 7};
        utils::print(results);
        EXPECT_EQ(results, expectedResults);
    }

    // Generate a dot graph for a test graph.
    {
        std::stringstream writer;
        utils::graph::graph_info(g, writer);
        fmt::print("{}\n", writer.str());
        std::vector<std::string> v = get_vertex_ids();
        std::string dotFile("test.dot");
        utils::graph::gendot(g, v, dotFile);
        // utils::graph::viewdot(dotFile);
    }
}

template <typename IArchive, typename OArchive> void test_cereal() {
    auto edges = createTestData<int>();
    std::cout << "==== Edge information ====\n";
    utils::print(edges);
    utils::SparseGraph<int, int> g(edges, vertex_num(edges), true);
    auto v = g.getVertexes();
    auto e = g.getEdges();

    fmt::MemoryWriter writer;
    utils::graph::graph_info(g, writer);
    fmt::print("{}\n", writer.str());
    std::vector<std::string> vids = get_vertex_ids();

    // Serialize the data to output string stream
    std::ostringstream os;

    {
        OArchive output(os);
        utils::save_sparse_graph(output, g, vids);
    }

    // Deserialize the data from a string stream
    decltype(g)::VertexContainer v_read;
    decltype(g)::EdgeContainer e_read;
    std::vector<std::string> vids_read;

    std::istringstream is(os.str());
    IArchive input(is);
    input(vids_read, v_read, e_read);

    utils::print(vids_read);
    utils::print(v_read);
    utils::print(e_read);

    EXPECT_EQ(v, v_read);
    EXPECT_EQ(e, e_read);
    EXPECT_EQ(vids, vids_read);
}

TEST(SerializationUsingCereal, Positive) {
    using OArchive = cereal::BinaryOutputArchive;
    using IArchive = cereal::BinaryInputArchive;
    test_cereal<IArchive, OArchive>();
    // test_cereal<cereal::JSONInputArchive, cereal::JSONOutputArchive>();
}
