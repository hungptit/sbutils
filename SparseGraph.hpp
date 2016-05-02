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
      size_t numberOfVertexes() const {
        return Vertexes.size() - 1;
      }
      
      private:
        vertex_container Vertexes;
        edge_container Edges;
        bool IsDirected;
    };

    // Utilty functions
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
