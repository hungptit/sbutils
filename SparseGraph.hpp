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

#include "cppformat/format.h"
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
        using EdgeContainer = std::vector<index_type>;
        using VertexContainer = std::vector<index_type>;
        using ColorContainer = std::vector<int>;
        using edge_iterator = typename EdgeContainer::const_iterator;
        using vetex_iterator = typename VertexContainer::const_iterator;

        explicit SparseGraph(const bool isDirected) : IsDirected(isDirected) {}

        explicit SparseGraph(std::vector<std::tuple<index_type, index_type>> &data,
                             const std::size_t N, const bool isDirected)
            : IsDirected(isDirected) {

            build(data, N);
        }

        explicit SparseGraph(VertexContainer &v, EdgeContainer &e,
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
        const VertexContainer &getVertexes() const { return Vertexes; }
        const EdgeContainer &getEdges() const { return Edges; }
        bool isDirected() { return IsDirected; };
      
      private:
        VertexContainer Vertexes;
        EdgeContainer Edges;
        bool IsDirected;
    };

    namespace graph {
        enum Status { UNDISCOVERED, VISITED, DISCOVERED };

        /**
         * Below classes are implementation for vertex visitors that can be used
         * with standard graph algorithms such as dfs and bfs.
         * 
         */

        template <typename Graph, typename Container> class NormalVisitor {
          public:
            using index_type = typename Graph::index_type;
            using edge_container = typename Graph::EdgeContainer;
            using results_container = std::vector<index_type>;

            explicit NormalVisitor(const size_t N) : States(N, UNDISCOVERED) {}

            const results_container &getResults() const { return Results; }

            void visit(const Graph &g, Container &stack, index_type &vid) {
                // std::cout << "Visit " << vid << "\n";
                auto const &vertexes = g.getVertexes();
                auto const &edges = g.getEdges();
                States[vid] = VISITED;
                for (auto idx = vertexes[vid]; idx < vertexes[vid + 1]; idx++) {
                    stack.push_back(getChildVid(edges, idx));
                }
                Results.push_back(vid);
                States[vid] = DISCOVERED;
            }

            bool isUndiscovered(const index_type vid) const {
                return (States[vid] == UNDISCOVERED);
            }

          private:
            std::vector<Status> States;
            results_container Results;
            index_type getChildVid(const edge_container &edges, size_t idx) {
                return edges[idx];
            }
        };

        /// Display the graph information to stdout
        template <typename Graph> void graph_info(Graph &g) {
            auto vertexes = g.getVertexes();
            auto edges = g.getEdges();
            fmt::MemoryWriter writer;
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
            std::cout << writer.str() << std::endl;
        }

        //@{ Graph algorithms //@}

        template <typename Visitor, typename Graph>
        void dfs(Graph &g, Visitor &visitor,
                 std::vector<typename Graph::index_type> vids) {
            using index_type = typename Graph::index_type;
            std::vector<index_type> stack(vids.begin(), vids.end());
            while (!stack.empty()) {
                auto vid = stack.back();
                stack.pop_back();
                if (visitor.isUndiscovered(vid)) {
                    visitor.visit(g, stack, vid);
                }
            }
        }

        /// Write input graph information to a dot file.
        template <typename Graph>
        void gendot(Graph &g, std::vector<std::string> &v,
                    const std::string &dotfile) {
            fmt::MemoryWriter writer;
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

        /// View a dot file using xdot
        int viewdot(const std::string &dotFile) {
            std::string cmd = fmt::format("xdot {0}&", dotFile);
            return std::system(cmd.c_str());
        }

        template <typename Graph>
        void tree_info(const Graph &g, std::vector<std::string> &vids) {
            utils::graph::graph_info(g);
            fmt::MemoryWriter writer;
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
