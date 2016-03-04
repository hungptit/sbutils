#ifndef Graph_hpp_
#define Graph_hpp_

#include <tuple>
#include <vector>

namespace utils {
    // Generate a dot graph
    template <typename Container>
    std::string gendot(Container &Column, Container &Row) {}

    template <typename Container>
    void viewdot(Container &Column, Container &Row) {
        // Create a temporary folder.
        // Create a temporary dot file
        // Generate the graph
        // View the graph
    }

    // Check that the graph information are correct.
    template <typename Container>
    std::vector<typename Container::value_type> validate(Container &Column,
                                                         Container &Row) {}
    // DFS algorithm
    template <typename Container>
    std::vector<typename Container::value_type>
    dfs(Container &Column, Container &Row, Container::value_type &v) {}

    // BFS algorithm
    template <typename Container>
    std::vector<typename Container::value_type>
    bfs(Container &Column, Container &Row, Container::value_type &v) {}
}
#endif
