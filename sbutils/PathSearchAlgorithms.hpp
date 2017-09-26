#pragma once

namespace sbutils {
    /**
     * Search for files in given folders using depth-first-search algorithm.
     *
     * @param searchPaths
     * @param visitor
     *
     * @return
     */
    template <typename Container, typename Visitor>
    void dfs_file_search(const Container &searchPaths, Visitor &visitor) {
        Container folders(searchPaths);
        while (!folders.empty()) {
            auto aPath = folders.back();
            folders.pop_back();
            visitor.visit(aPath, folders);
        }
    }

    /**
     * Search for files in given folders using breath-first-search
     * algorithm.
     *
     * @param searchPaths
     * @param visitor
     *
     * @return
     */
    template <typename Visitor, typename Container>
    void bfs_file_search(const Container &searchPaths, Visitor &visitor) {
        Container folders(searchPaths);
        while (!folders.empty()) {
            auto aPath = folders.front();
            folders.pop_front();
            visitor.visit(aPath, folders);
        }
    }
} // namespace filesystem
