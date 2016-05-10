#ifndef DFS_HPP_
#define DFS_HPP_

namespace graph {
    template <typename Graph, typename Visitor>
    auto preorder_dfs(Graph &g, Visitor &v, const std::vector<typename Graph::index_type> &vids) {
        using index_type = typename Graph::index_type;
        std::vector<index_type> stack(vids.begin(), vids.end());
        std::vector<index_type> results;
        Visitor visitor(g);
        while (!stack.empty()) {
            results.push_back(vid);
            visitor.visited(vid);
            visitor.visit(stack, vid);
        }
    }
}

#endif
