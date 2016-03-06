#include "utils/Utils.hpp"
#include "utils/SparseGraph.hpp"
#include <iostream>
#include <tuple>
#include <vector>

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

int main() {
    auto edges = createTestData<int>();
    std::cout << "==== Edge information ====\n";
    utils::print(edges);
    utils::SparseGraph<int> g(edges, vertex_num(edges), true);

    std::cout << "==== Obtained edge information ====\n";
    auto e = g.edges(0);
    std::vector<int> results(std::get<0>(e), std::get<1>(e));
    utils::print(results);

    std::cout << "==== Graph information ====\n";
    utils::graph_info(g);

    std::vector<std::string> v{"-0-", "-1-", "-2-", "-3-",
                               "-4-", "-5-", "-6-", "-7-"};
    std::string dotFile("test.dot");
    utils::gendot(g, v, dotFile);
    utils::viewdot(dotFile);

    // DFS
    {
        utils::DFS<decltype(g)> alg;
        fmt::print("Visited vertexes\n");
        utils::print(alg.dfs(g, 0));

        fmt::print("Visited vertexes\n");
        utils::print(alg.dfs(g, 1));

        fmt::print("Visited vertexes\n");
        utils::print(alg.dfs(g, 7));

        fmt::print("Visited vertexes\n");
        utils::print(alg.dfs(g, 10));
    }

    // BFS
    {
        utils::BFS<decltype(g)> alg;
        fmt::print("Visited vertexes\n");
        utils::print(alg.bfs(g, 1));

        fmt::print("Visited vertexes\n");
        utils::print(alg.bfs(g, 7));

        fmt::print("Visited vertexes\n");
        utils::print(alg.bfs(g, 0));
    }
}
