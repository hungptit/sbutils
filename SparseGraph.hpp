/**
 * @file   SparseGraph.hpp
 * @author Hung Dang <hungptit@gmail.com>
 * @date   Fri Mar  4 18:46:58 2016
 *
 * @brief  This file has a simple implementation of a directed sparse graph.
 *
 */

#ifndef SparseGraph_hpp_
#define SparseGraph_hpp_

#include <deque>
#include <fstream>
#include <iostream>
#include <tuple>
#include <vector>

namespace utils {
    /**
     * This class has a simple data sructure for a directed graph.  @todo
     * Support weighted graph. Might need to introduce edge_type and vertex_type
     * to support weight.
     *
     */
    template <typename vtype, typename etype> class SparseGraph {
      public:
        using index_type = vtype;
        using edge_type = etype;
        using edge_container = std::vector<index_type>;
        using vertex_container = std::vector<index_type>;
        using edge_iterator = typename edge_container::const_iterator;

        explicit SparseGraph(const bool isDirected) : IsDirected(isDirected) {}

        explicit SparseGraph(
            std::vector<std::tuple<index_type, index_type>> &data,
            const std::size_t N, const bool isDirected)
            : IsDirected(isDirected) {

            build(data, N);
        }

        explicit SparseGraph(vertex_container &v, edge_container &e,
                             bool isDirected)
            : Vertexes(v), Edges(e), IsDirected(isDirected) {}

        /// This function assume that input data is sorted by vertex ID.
        void build(std::vector<std::tuple<index_type, index_type>> &data,
                   const std::size_t N) {
            Vertexes.assign(N + 1, 0);
            Edges.assign(data.size(), 0);
            index_type currentRow = 0;
            for (auto const &val : data) {
                Vertexes[std::get<0>(val) + 1]++;
                Edges[currentRow++] = std::get<1>(val);
            }

            for (std::size_t currentCol = 0; currentCol < N; ++currentCol) {
                Vertexes[currentCol + 1] += Vertexes[currentCol];
            }
        }

        std::tuple<edge_iterator, edge_iterator> edges(index_type vid) {
            return std::make_tuple(Edges.begin() + Vertexes[vid],
                                   Edges.begin() + Vertexes[vid + 1]);
        }
        const vertex_container &getVertexes() const { return Vertexes; }
        const edge_container &getEdges() const { return Edges; }
        bool isDirected() { return IsDirected; };

      private:
        vertex_container Vertexes;
        edge_container Edges;
        bool IsDirected;
    };

    namespace graph {
        enum Status { UNDISCOVERED, DISCOVERED, PROCESSED };

        template <typename Graph, typename Writer>
        void graph_info(Graph &g, Writer &writer) {
            auto vertexes = g.getVertexes();
            auto edges = g.getEdges();
            auto N = vertexes.size() - 1;
            for (std::size_t currentCol = 0; currentCol < N; ++currentCol) {
                auto begin = vertexes[currentCol];
                auto end = vertexes[currentCol + 1];
                writer << "Vetex[" << currentCol << "]: Edges = {";

                if (begin != end) {
                    writer << edges[begin];
                    begin++;
                }

                for (auto idx = begin; idx < end; ++idx) {
                    writer << "," << edges[idx];
                }
                writer << "}\n";
            }
        }

        /**
         * Below classes are implementations for vertex visitors that can be
         * used
         * with both dfs and bfs.
         *
         */

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
        auto dfs(Graph &g, std::vector<typename Graph::index_type> vids) {
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
        auto
        topological_sorted_list(Graph &g, std::vector<typename Graph::index_type> vids) {
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
        auto bfs(Graph &g, std::vector<typename Graph::index_type> vids) {
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

        /// Write input graph information to a dot file.
        template <typename Graph, typename Writer = std::stringstream>
        void gendot(Graph &g, std::vector<std::string> &v,
                    const std::string &dotfile) {
            Writer writer;
            writer << (g.isDirected() ? ("digraph") : ("graph"))
                   << (" G {\n"); // Header

            // Write vertex's information
            int i = 0;
            writer << "\t" << i << "[peripheries=2, label=\"" << v[i]
                   << "\",style=bold,shape=box]\n";
            for (auto const &val : v) {
                writer << "\t" << i << "[label=\"" << val << "\"]\n";
                i++;
            }

            auto vertexes = g.getVertexes();
            auto edges = g.getEdges();

            // Write edge's' information
            const std::string direction = g.isDirected() ? "->" : "--";
            std::size_t N = vertexes.size() - 1;
            for (std::size_t vid = 0; vid < N; ++vid) {
                auto begin = vertexes[vid];
                auto end = vertexes[vid + 1];
                for (auto it = begin; it != end; ++it) {
                    writer << "\t" << vid << direction << edges[it] << "\n";
                }
            }

            writer << "}\n"; // Footer

            // Write data to a dot file
            std::ofstream myfile;
            myfile.open(dotfile);
            myfile << writer.str();
        }

        /// View a dot file using dot command.
        void viewdot(const std::string &dotFile) {
            const std::string cmd = "dot -Txlib " + dotFile;
            std::system(cmd.c_str());
        }

        template <typename Graph, typename Writer = std::stringstream>
        void tree_info(const Graph &g, std::vector<std::string> &vids) {
            utils::graph::graph_info(g);
            Writer writer;
            size_t counter = 0;
            for (auto item : vids) {
                writer << "vid[" << counter << "] = " << item << "\n";
                counter++;
            }
            std::cout << writer.str();
        }
    }
}
#endif
