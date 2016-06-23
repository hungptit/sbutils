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

namespace graph {
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
            : VertexData(v), OutEdgeData(e), IsDirected(isDirected) {}

        /// This function assume that input data is sorted by vertex ID.
        void build(std::vector<std::tuple<index_type, index_type>> &data,
                   const std::size_t N) {
            VertexData.assign(N + 1, 0);
            OutEdgeData.assign(data.size(), 0);
            index_type currentRow = 0;
            for (auto const &val : data) {
                VertexData[std::get<0>(val) + 1]++;
                OutEdgeData[currentRow++] = std::get<1>(val);
            }

            for (std::size_t currentCol = 0; currentCol < N; ++currentCol) {
                VertexData[currentCol + 1] += VertexData[currentCol];
            }
        }


        bool isDirected() { return IsDirected; };
        const vertex_container &vertexData() const { return VertexData; }
        const edge_container &outEdgeData() const { return OutEdgeData; }
        const edge_container &inEdgeData() const { return InEdgeData; }
        size_t numberOfVertices() const { return VertexData.size() - 1; }
        auto edges(const index_type vid) const {
            return std::make_tuple(OutEdgeData.begin() + VertexData[vid], OutEdgeData.begin() + VertexData[vid + 1]);
        }

      private:
        vertex_container VertexData;
        edge_container OutEdgeData;
        edge_container InEdgeData;
        bool IsDirected;
    };   

    // Utilty functions    
    template <typename Graph, typename Writer>
    void graph_info(Graph &g, Writer &writer) {
        auto vertexes = g.vertexData();
        auto edges = g.outEdgeData();
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

        auto vertexes = g.vertexData();
        auto edges = g.outEdgeData();

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
        graph_info(g);
        Writer writer;
        size_t counter = 0;
        for (auto item : vids) {
            writer << "vid[" << counter << "] = " << item << "\n";
            counter++;
        }
        std::cout << writer.str();
    }
}
#endif
