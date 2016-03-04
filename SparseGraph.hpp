/**
 * @file   SparseGraph.hpp
 * @author Hung Dang <hungptit@gmail.com>
 * @date   Fri Mar  4 18:46:58 2016
 *
 * @brief  This file has a simple implementation of a directed sparse graph.
 *
 *
 */

#ifndef SparseGraph_hpp_
#define SparseGraph_hpp_

#include "cppformat/format.h"
#include <fstream>
#include <iostream>
#include <tuple>
#include <vector>

namespace utils {

    /**
     * This class has a simple data sructure for a directed graph.
     *
     */
    template <typename Index> class DirectedSparseGraph {
      public:
        using EdgeContainer = std::vector<Index>;
        using VertexContainer = std::vector<Index>;
        using edge_iterator = typename EdgeContainer::const_iterator;
        using vetex_iterator = typename VertexContainer::const_iterator;

        /// Construct an empty graph
        explicit DirectedSparseGraph() {}

        /// Construct a graph given edge's information.
        explicit DirectedSparseGraph(
            std::vector<std::tuple<Index, Index>> &data, const std::size_t N) {
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
        bool isDirected() { return true; };

      protected:
        VertexContainer Vertexes;
        EdgeContainer Edges;
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
        writer << i << "[label=\"" << v[i] << "\",style=bold,shape=box]\n";
        for (auto const &val : v) {
            writer << i << "[label=\"" << val << "\"]\n";
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
                writer << vid << direction << edges[it] << "\n";
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
