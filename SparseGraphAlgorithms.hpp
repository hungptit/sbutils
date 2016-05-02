#ifndef SparseGraphAlgorithms_hpp_
#define SparseGraphAlgorithms_hpp_

#include "SparseGraph.hpp"
#include <vector>
#include "Print.hpp"

namespace utils {
    namespace graph {
        enum Status { UNDISCOVERED, DISCOVERED, PROCESSED };

        template <typename Graph, typename Container> class Visitor {
          public:
            using index_type = typename Graph::index_type;
            using edge_container = typename Graph::edge_container;
            using results_container = std::vector<index_type>;

            explicit Visitor(Graph &g)
                : States(g.getVertexes().size() - 1, UNDISCOVERED),
                  Vertexes(g.getVertexes()), Edges(g.getEdges()){};

            void visit(Container &stack, const index_type &vid) {
                auto const lower_bound = Vertexes[vid];
                auto const upper_bound = Vertexes[vid + 1];
                for (auto idx = lower_bound; idx < upper_bound; ++idx) {
                    auto const currentVid = Edges[idx];
                    if (!isVisited(currentVid)) {
                        stack.push_back(currentVid);
                    }
                }
            }

            bool isVisited(const index_type vid) {
                return States[vid] == PROCESSED;
            }

            void visited(const index_type vid) { States[vid] = PROCESSED; }

          private:
            std::vector<Status> States;
            const typename Graph::vertex_container Vertexes;
            const typename Graph::edge_container Edges;
        };

        // This is a simple DFS algorithm
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
        auto connected_components(
            Graph &g, Visitor &visitor) {
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
        auto strongly_connected_components(
            Graph &g, Visitor &visitor) {
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
}

#endif
