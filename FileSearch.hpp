#ifndef FileSearch_hpp_
#define FileSearch_hpp_

// STL headers
#include <array>
#include <string>
#include <vector>
#include <unordered_map>

// Boost libraries
#define BOOST_THREAD_VERSION 4
#include "boost/config.hpp"
#include "boost/filesystem.hpp"

#include "cppformat/format.h"

#include "SparseGraph.hpp"
#include "Utils.hpp"

namespace utils {
    struct DonotFilter {
        bool isValidStem(const std::string &) { return true; }
        bool isValidExt(const std::string &) { return true; }
    };

    class NormalFilter {
      public:
        bool isValidExt(const std::string &anExtension) {
            return std::find(ExcludedExtensions.begin(),
                             ExcludedExtensions.end(),
                             anExtension) == ExcludedExtensions.end();
        }

        bool isValidStem(const std::string &aStem) {
            return std::find(ExcludedStems.begin(), ExcludedStems.end(),
                             aStem) == ExcludedStems.end();
        }

      private:
        const std::array<std::string, 1> ExcludedExtensions = {{".git"}};
        const std::array<std::string, 2> ExcludedStems = {
            {"CMakeFiles", "CMakeTmp"}};
    };

    class MWFilter {
      public:
        bool isValidExt(const std::string &anExtension) {
            return std::find(ExcludedExtensions.begin(),
                             ExcludedExtensions.end(),
                             anExtension) == ExcludedExtensions.end();
        }

        bool isValidStem(const std::string &aStem) {
            return std::find(ExcludedStems.begin(), ExcludedStems.end(),
                             aStem) == ExcludedStems.end();
        }

      private:
        const std::array<std::string, 1> ExcludedExtensions = {{".sbtools"}};
        const std::array<std::string, 2> ExcludedStems = {{"doc", "doxygen"}};
    };

    template <typename PathContainer, typename Filter> class Visitor {
      public:
        using container_type = std::vector<FileInfo>;
        using path = boost::filesystem::path;
        using directory_iterator = boost::filesystem::directory_iterator;
        using path_container = std::vector<path>;
        using vertex_type = std::tuple<std::string, container_type>;
        using Index = int;

        void visit(path &aPath, PathContainer &folders) {
            namespace fs = boost::filesystem;
            directory_iterator endIter;
            directory_iterator dirIter(aPath);
            for (; dirIter != endIter; ++dirIter) {
                auto currentPath = dirIter->path();
                auto status = dirIter->status();
                auto ftype = status.type();
                std::string currentPathStr = currentPath.string();
                auto aStem = currentPath.stem().string();
                auto anExtension = currentPath.extension().string();
                if (ftype == boost::filesystem::regular_file) {
                    vertex_data.emplace_back(std::make_tuple(
                        currentPathStr, aStem, currentPath.extension().string(),
                        status.permissions(), fs::last_write_time(aPath),
                        fs::file_size(currentPath)));
                } else if (ftype == boost::filesystem::directory_file) {
                    if (CustomFilter.isValidStem(aStem) &&
                        CustomFilter.isValidExt(anExtension)) {
                        edges.emplace_back(std::make_tuple(
                            aPath.string(), currentPath.string()));
                        folders.emplace_back(currentPath);
                    }
                } else {
                }
            }

            vertexes.emplace_back(aPath.string(), vertex_data);
            vertex_data.clear();
        }

        void print() {
            size_t counter = 0;
            for (auto const &item : vertexes) {
                counter += std::get<1>(item).size();
            }
            fmt::print("Number of vertexes: {0}\n", vertexes.size());
            fmt::print("Number of edgess: {0}\n", edges.size());
            fmt::print("Number of files: {0}\n", counter);
        }

        auto compact() {
            using Index = int;

            // Create a lookup table
            std::sort(vertexes.begin(), vertexes.end());
            std::vector<std::pair<std::string, Index>> values;
            values.reserve(vertexes.size());
            Index counter = 0;
            for (auto item : vertexes) {
                auto aPath = std::get<0>(item);
                values.push_back(std::make_pair(aPath, counter));
                counter++;
            }
            std::unordered_map<std::string, Index> lookupTable(values.begin(),
                                                               values.end());

            // Prepare the input for our folder hierarchy graph
            std::vector<std::tuple<Index, Index>> allEdges;
            allEdges.reserve(edges.size());
            for (auto anEdge : edges) {
                allEdges.push_back(
                    std::make_tuple(lookupTable[std::get<0>(anEdge)],
                                    lookupTable[std::get<1>(anEdge)]));
            }
            std::sort(allEdges.begin(), allEdges.end());

            // Return vertex information and a folder hierarchy graph.
            return std::make_tuple(
                vertexes, SparseGraph<Index>(allEdges, vertexes.size(), true));
        }

      private:
        // Temporary variable. Should be on top to improve the performance.
        container_type vertex_data;
        Filter CustomFilter;

        // Information about the folder hierarchy.
        using edge_type = std::tuple<std::string, std::string>;
        std::vector<edge_type> edges;
        std::vector<vertex_type> vertexes;

        // Data that will be used to filter search space.
        std::array<std::string, 1> ExcludedExtensions = {{".git"}};
        std::array<std::string, 1> ExcludedStems = {{"CMakeFiles"}};
    };

    template <typename PathContainer, typename Filter> class SimpleVisitor {
      public:
        using container_type = std::vector<FileInfo>;
        using path = boost::filesystem::path;
        using directory_iterator = boost::filesystem::directory_iterator;
        using path_container = std::vector<path>;

        const container_type &getResults() const { return Results; }

        void visit(path &aPath, PathContainer &folders) {
            namespace fs = boost::filesystem;
            directory_iterator endIter;
            directory_iterator dirIter(aPath);
            for (; dirIter != endIter; ++dirIter) {
                auto currentPath = dirIter->path();
                auto status = dirIter->status();
                auto ftype = status.type();
                auto aStem = currentPath.stem().string();
                auto anExtension = currentPath.extension().string();
                if (ftype == boost::filesystem::regular_file) {
                    Results.emplace_back(std::make_tuple(
                        currentPath.string(), aStem, currentPath.extension().string(),
                        status.permissions(), fs::last_write_time(aPath),
                        fs::file_size(currentPath)));
                } else if (ftype == boost::filesystem::directory_file) {
                    if (CustomFilter.isValidStem(aStem) &&
                        CustomFilter.isValidExt(anExtension)) {
                        folders.emplace_back(currentPath);
                    }
                } else {
                  // Donot do anything here
                }
            }
        }

      private:
        Filter CustomFilter;
        std::array<std::string, 1> ExcludedExtensions = {{".git"}};
        std::array<std::string, 1> ExcludedStems = {{"CMakeFiles"}};
        std::vector<FileInfo> Results;
    };

    /**
     * Search for files in given folders using depth-first-search algorithm.
     *
     * @param searchPaths
     * @param visitor
     *
     * @return
     */
    template <typename Visitor, typename Container>
    size_t dfs_file_search(Container &searchPaths, Visitor &visitor) {
        using path = boost::filesystem::path;
        size_t counter = 0;
        std::vector<path> folders(searchPaths.begin(), searchPaths.end());
        while (!folders.empty()) {
            auto aPath = folders.back();
            folders.pop_back();
            visitor.visit(aPath, folders);
        }
        return counter;
    }

    /**
     * Search for files in given folders using breath-first-search algorithm.
     *
     * @param searchPaths
     * @param visitor
     *
     * @return
     */
    template <typename Visitor, typename Container>
    size_t bfs_file_search(Container &searchPaths, Visitor &visitor) {
        using path = boost::filesystem::path;
        size_t counter = 0;
        Container folders(searchPaths.begin(), searchPaths.end());
        while (!folders.empty()) {
            auto aPath = folders.front();
            folders.pop_front();
            visitor.visit(aPath, folders);
        }
        return counter;
    }

    //   /**
    //      * Find all file using the algorithm provided by the base class.
    //      *
    //      */
    // using FileInfo = std::tuple<std::string, std::string, std::string, int,
    // std::time_t>;

    //   template <typename Base> class FileSearchBase : public Base {
    //       public:
    //         explicit FileSearchBase() {}

    //         explicit FileSearchBase(size_t size) { Data.reserve(size); }

    //         auto getData() { return Data; }

    //         template <typename StemContainer, typename ExtContainer>
    //         std::vector<FileInfo> filter(StemContainer &stems,
    //                                             ExtContainer &exts) {
    //           std::vector<FileInfo> results;
    //             for (auto item : Data) {
    //                 auto stem = std::get<1>(item);
    //                 auto ext = std::get<2>(item);
    //                 bool flag = (std::find(stems.begin(), stems.end(),
    //                                        std::get<1>(item)) == stems.end())
    //                                        &&
    //                             ((std::find(exts.begin(), exts.end(),
    //                                         std::get<2>(item)) ==
    //                                         exts.end()));
    //                 if (flag) {
    //                     results.emplace_back(item);
    //                 }
    //             }
    //             return results;
    //         }

    //       protected:
    //         typedef typename Base::path path;
    //         bool isValidDir(const path &aPath) {
    //             const std::array<std::string, 1> excludedFolders =
    //             {{".git"}};
    //             auto searchStr = aPath.extension().string();
    //             return std::find(excludedFolders.begin(),
    //             excludedFolders.end(),
    //                              searchStr) == excludedFolders.end();
    //         };

    //         bool isValidFile(const path &) { return true; };

    //         void update(const path &aPath, boost::filesystem::file_status
    //         &fs) {
    //             Data.emplace_back(
    //                 std::make_tuple(aPath.string(), aPath.stem().string(),
    //                                 aPath.extension().string(),
    //                                 fs.permissions(),
    //                                 boost::filesystem::last_write_time(aPath)));
    //         };

    //         void unexpected(const path &){
    //             // TODO: How could we handle symlink files?
    //         };

    //       private:
    //     std::vector<FileInfo> Data;
    //     };

    //     template <typename Base, typename StemContainer, typename
    //     ExtContainer>
    //     class BasicFileSearch : public Base {
    //       public:
    //         BasicFileSearch(StemContainer &stems, ExtContainer &exts)
    //             : Stems(stems), Exts(exts) {}

    //         auto getData() { return Data; }

    //       protected:
    //         typedef typename Base::path path;

    //         bool isValidDir(const path &aPath) {
    //             const std::array<std::string, 1> excludedFolders =
    //             {{".git"}};
    //             auto searchStr = aPath.extension().string();
    //             return std::find(excludedFolders.begin(),
    //             excludedFolders.end(),
    //                              searchStr) == excludedFolders.end();
    //         };

    //         bool isValidFile(const path &aPath) {
    //             return (Stems.empty() ||
    //                     (std::find(Stems.begin(), Stems.end(),
    //                                aPath.stem().string()) != Stems.end())) &&
    //                    (Exts.empty() ||
    //                     (std::find(Exts.begin(), Exts.end(),
    //                                aPath.extension().string()) !=
    //                                Exts.end()));
    //         };

    //         void update(const path &aPath, boost::filesystem::file_status
    //         &fs) {
    //             Data.emplace_back(
    //                 std::make_tuple(aPath.string(), aPath.stem().string(),
    //                                 aPath.extension().string(),
    //                                 fs.permissions(),
    //                                 boost::filesystem::last_write_time(aPath)));
    //         };

    //         void unexpected(const path &){
    //             // TODO: How could we handle symlink files?
    //         };

    //       private:
    //       std::vector<FileInfo> Data;
    //         StemContainer Stems;
    //         ExtContainer Exts;
    //     };
}

#endif
