/**
 * @file   SparseGraph.hpp
 * @author Hung Dang <hungptit@gmail.com>
 * @date   Fri Mar  4 18:46:58 2016
 *
 * @brief  This file has a simple implementation of a directed sparse graph.
 *
 * @note Sparse graph class will provide the data structure only.
 * @note Graph algorithms will operate on vertexes and edges information.
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
    enum VertexStatus { UNDISCOVERED, DISCOVERED, VISITED };

    /**
     * This class has a simple data sructure for a directed graph.
     * @todo Support weighted graph.
     */
    template <typename Index> class SparseGraph {
      public:
        using index_type = Index;
        using EdgeContainer = std::vector<Index>;
        using VertexContainer = std::vector<Index>;
        using ColorContainer = std::vector<int>;
        using edge_iterator = typename EdgeContainer::const_iterator;
        using vetex_iterator = typename VertexContainer::const_iterator;

        /// Construct an empty graph
        explicit SparseGraph(const bool isDirected) : IsDirected(isDirected) {}

        /// Construct a graph given edge's information.
        explicit SparseGraph(std::vector<std::tuple<Index, Index>> &data,
                             const std::size_t N, const bool isDirected)
            : IsDirected(isDirected) {

            build(data, N);
        }

        /// This function assume that input data is sorted by vertex ID.
        void build(std::vector<std::tuple<Index, Index>> &data,
                   const std::size_t N) {
            Vertexes.assign(N + 1, 0);
            Edges.assign(data.size(), 0);
            Index currentRow = 0;
            for (auto const &val : data) {
                Vertexes[std::get<0>(val) + 1]++;
                Edges[currentRow++] = std::get<1>(val);
            }

            for (std::size_t currentCol = 0; currentCol < N; ++currentCol) {
                Vertexes[currentCol + 1] += Vertexes[currentCol];
            }
        }

        /// Return  the range of edges for a given vertex id.
        std::tuple<edge_iterator, edge_iterator> edges(Index vid) {
            return std::make_tuple(Edges.begin() + Vertexes[vid],
                                   Edges.begin() + Vertexes[vid + 1]);
        }
        const VertexContainer &getVertexes() const { return Vertexes; }
        const EdgeContainer &getEdges() const { return Edges; }
        bool isDirected() { return IsDirected; };

      protected:
        bool IsDirected = true;
        VertexContainer Vertexes;
        EdgeContainer Edges;
    };

    template <typename Graph> class DFS {
      public:
        using Index = typename Graph::index_type;

        auto dfs(Graph &g, const Index vid) {
            auto vertexes = g.getVertexes();
            auto edges = g.getEdges();
            std::vector<VertexStatus> status(vertexes.size(), UNDISCOVERED);
            std::vector<Index> results;
            if (static_cast<std::size_t>(vid) < vertexes.size()) {
                Stack.push_back(vid);
                while (!Stack.empty()) {
                    auto currentVid = Stack.back();
                    Stack.pop_back();
                    if (status[currentVid] == UNDISCOVERED) {
                        visit(vertexes, edges, status, currentVid, results);
                    }                    
                }
            } else {
                std::cerr << "Invalid of value for vertex id: " << vid << "\n";
            }

            // Return a list of visited vertexes.
            return results;
        }

      protected:
        std::vector<Index> Stack;

        void visit(typename Graph::VertexContainer &vertexes,
                   typename Graph::EdgeContainer &edges,
                   std::vector<VertexStatus> &status, Index vid,
                   std::vector<Index> &results) {
            const auto begin = vertexes[vid];
            const auto end = vertexes[vid + 1];

            // Visit a given node
            status[vid] = VISITED;
            for (auto it = begin; it != end; ++it) {
                auto childVid = edges[it];
                if (status[childVid] == UNDISCOVERED) {
                    Stack.push_back(childVid);
                }
            }
            status[vid] = DISCOVERED;
            results.push_back(vid);
        }
    };

    template <typename Graph> class BFS {
      public:
        using Index = typename Graph::index_type;

        auto bfs(Graph &g, const Index vid) {
            auto vertexes = g.getVertexes();
            auto edges = g.getEdges();
            std::vector<VertexStatus> status(vertexes.size(), UNDISCOVERED);
            std::vector<Index> results;
            if (static_cast<std::size_t>(vid) < vertexes.size()) {
                Queue.push_back(vid);
                while (!Queue.empty()) {
                    auto currentVid = Queue.front();
                    Queue.pop_front();
                    if (status[currentVid] == UNDISCOVERED) {
                        visit(vertexes, edges, status, currentVid, results);
                    }
                }
            } else {
                std::cerr << "Invalid of value for vertex id: " << vid << "\n";
            }

            // Return a list of visited vertexes.
            return results;
        }

      protected:
        std::deque<Index> Queue;

        void visit(typename Graph::VertexContainer &vertexes,
                   typename Graph::EdgeContainer &edges,
                   std::vector<VertexStatus> &status, Index vid,
                   std::vector<Index> &results) {
            const auto begin = vertexes[vid];
            const auto end = vertexes[vid + 1];

            // Visit a given node
            status[vid] = VISITED;
            for (auto it = begin; it != end; ++it) {
                auto childVid = edges[it];
                if (status[childVid] == UNDISCOVERED) {
                    Queue.push_back(childVid);
                }
            }
            status[vid] = DISCOVERED;
            results.push_back(vid);
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
    void viewdot(const std::string &dotFile) {
        std::string cmd = fmt::format("xdot {0}&", dotFile);
        std::system(cmd.c_str());
    }
}
#endif
