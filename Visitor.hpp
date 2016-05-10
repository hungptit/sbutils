#ifndef VISITOR_HPP
#define VISITOR_HPP
namespace Graph {
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

        auto status(const index_type vid) {
            return States[vid];
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
}
#endif
