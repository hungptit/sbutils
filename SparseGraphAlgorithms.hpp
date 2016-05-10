#ifndef SparseGraphAlgorithms_hpp_
#define SparseGraphAlgorithms_hpp_

#include "Print.hpp"
#include "SparseGraph.hpp"
#include <vector>

namespace graph {
    // DFS algorithm that use the default visitor.
    template <typename Graph, typename Visitor>
    auto dfs(Graph &g,
             const std::vector<typename Graph::index_type> &vids) {
        using index_type = typename Graph::index_type;

        // Initialize DFS
        std::vector<index_type> stack(vids.begin(), vids.end());
        std::vector<index_type> results;
        Visitor visitor(g);

        // Traverse the graph.
        while (!stack.empty()) {
            auto const vid = stack.back();
            stack.pop_back();
            if (!visitor.isVisited(vid)) {
                results.push_back(vid);
                visitor.visited(vid);
                visitor.visit(stack, vid);
            }
        }

        return results;
    }

    // DFS algorithm that uses provided visitor.
    template <typename Graph, typename Visitor>
    auto dfs(Graph &, Visitor &visitor,
             const std::vector<typename Graph::index_type> &vids) {
        using index_type = typename Graph::index_type;
            
        // Initialize DFS
        std::vector<index_type> stack(vids.begin(), vids.end());
        std::vector<index_type> results;

        // Traverse the graph.
        while (!stack.empty()) {
            auto const vid = stack.back();
            stack.pop_back();
            if (!visitor.isVisited(vid)) {
                results.push_back(vid);
                visitor.visited(vid);
                visitor.visit(stack, vid);
            }
        }

        return results;
    }

    template <typename Graph, typename Visitor>
    auto connected_components(Graph &g, Visitor &visitor) {
        using index_type = typename Graph::index_type;
        std::vector<std::vector<index_type>> results;
        index_type N = g.numberOfVertexes();
        for (index_type vid = 0; vid < N; ++vid) {
            std::cout << "vid = " << vid << "\n";
            if (!visitor.isVisited(vid)) {
                results.emplace_back(dfs(g, visitor, {vid}));
            }
        }
        return results;
    }

    template <typename Graph, typename Visitor>
    auto strongly_connected_components(Graph &g, Visitor &visitor) {
        using index_type = typename Graph::index_type;
        std::vector<std::vector<index_type>> results;
        index_type N = g.numberOfVertexes();
        for (index_type vid = 0; vid < N; ++vid) {
            std::cout << "vid = " << vid << "\n";
            if (!visitor.isVisited(vid)) {
                results.emplace_back(dfs(g, visitor, {vid}));
            }
        }
        return results;
    }

    template <typename Graph, typename Visitor>
    auto topological_sorted_list(
        Graph &g, const std::vector<typename Graph::index_type> &vids) {
        using index_type = typename Graph::index_type;

        // Initialize DFS
        std::vector<index_type> stack(vids.begin(), vids.end());
        std::vector<index_type> results;
        Visitor visitor(g);

        // Traverse the graph.
        while (!stack.empty()) {
            auto const vid = stack.back();
            stack.pop_back();
            if (!visitor.isVisited(vid)) {
                results.push_back(vid);
                visitor.visited(vid);
                visitor.visit(stack, vid);
            }
        }

        return results;
    }

    template <typename Graph, typename Visitor>
    auto bfs(Graph &g,
             const std::vector<typename Graph::index_type> &vids) {
        using index_type = typename Graph::index_type;
        // utils::ElapsedTime<MILLISECOND> t("DFS time: ");

        // Initialize DFS
        std::deque<index_type> queue(vids.begin(), vids.end());
        std::vector<index_type> results;
        Visitor visitor(g);

        // Traverse the graph.
        while (!queue.empty()) {
            auto const vid = queue.front();
            queue.pop_front();
            if (!visitor.isVisited(vid)) {
                results.push_back(vid);
                visitor.visited(vid);
                visitor.visit(queue, vid);
            }
        }

        return results;
    }
}

#endif
