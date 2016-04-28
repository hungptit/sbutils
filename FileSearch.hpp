#ifndef FileSearch_hpp_
#define FileSearch_hpp_

// STL headers
#include <array>
#include <string>
#include <unordered_map>
#include <vector>

// Boost libraries
#define BOOST_THREAD_VERSION 4
#include "boost/config.hpp"
#include "boost/filesystem.hpp"

#include "fmt/format.h"

#include "SparseGraph.hpp"
#include "Utils.hpp"

namespace utils {
    namespace filesystem {
        enum {
            PATH = 0,
            STEM = 1,
            EXTENSION = 2,
            PERMISSION = 3,
            TIMESTAMP = 4,
            FILESIZE = 5
        };

        template <typename Container> class ExtFilter {
          public:
            explicit ExtFilter(Container &exts) : Extensions(exts) {}

            bool isValid(FileInfo &info) {
                if (Extensions.empty()) {
                    return true;
                } else {
                    return (std::find(Extensions.begin(), Extensions.end(),
                                      std::get<EXTENSION>(info)) !=
                            Extensions.end());
                }
            }

          private:
            Container Extensions;
        };

        template <typename Container> class StemFilter {
          public:
            explicit StemFilter(Container &stems) : Stems(stems) {}

            bool isValid(FileInfo &info) {
                if (Stems.empty()) {
                    return true;
                } else {

                    return (std::find(Stems.begin(), Stems.end(),
                                      std::get<STEM>(info)) != Stems.end());
                }
            }

          private:
            std::vector<std::string> Stems;
        };

        template <typename Iterator, typename Filter1>
        std::vector<utils::FileInfo> filter(Iterator begin, Iterator end,
                                            Filter1 &f1) {
            std::vector<utils::FileInfo> results;
            for (auto it = begin; it != end; ++it) {
                if (f1.isValid(*it)) {
                    results.emplace_back(*it);
                }
            }
            return results;
        }

        template <typename Iterator, typename Filter1, typename Filter2>
        std::vector<utils::FileInfo> filter(Iterator begin, Iterator end,
                                            Filter1 &f1, Filter2 &f2) {
            std::vector<utils::FileInfo> results;
            for (auto it = begin; it != end; ++it) {
                if (f1.isValid(*it) && f2.isValid(*it)) {
                    results.emplace_back(*it);
                }
            }
            return results;
        }

        struct DoNothingPolicy {
            bool isValidStem(const std::string &) { return true; }
            bool isValidExt(const std::string &) { return true; }
        };

        class NormalPolicy {
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

        class MWPolicy {
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
            const std::array<std::string, 1> ExcludedExtensions = {
                {".sbtools"}};
            const std::array<std::string, 2> ExcludedStems = {
                {"doc", "doxygen"}};
        };

        template <typename PathContainer, typename Filter> class Visitor {
          public:
            using container_type = std::vector<FileInfo>;
            using path = boost::filesystem::path;
            using directory_iterator = boost::filesystem::directory_iterator;
            using path_container = std::vector<path>;
            using vertex_type = std::tuple<std::string, container_type>;
            using index_type = int;

            void visit(path &aPath, PathContainer &folders) {
                namespace fs = boost::filesystem;
                directory_iterator endIter;
                directory_iterator dirIter(aPath);
                for (; dirIter != endIter; ++dirIter) {
                    auto currentPath = dirIter->path();
                    auto status = dirIter->status();
                    auto ftype = status.type();
                    boost::system::error_code errcode;
                    std::string currentPathStr = currentPath.string();
                    auto aStem = currentPath.stem().string();
                    auto anExtension = currentPath.extension().string();
                    if (ftype == boost::filesystem::regular_file) {
                        vertex_data.emplace_back(std::make_tuple(
                            currentPathStr, aStem,
                            currentPath.extension().string(),
                            status.permissions(),
                            fs::last_write_time(aPath, errcode),
                            fs::file_size(currentPath, errcode)));
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
                using index_type = int;

                // Create a lookup table
                std::sort(vertexes.begin(), vertexes.end());
                std::vector<std::pair<std::string, index_type>> values;
                values.reserve(vertexes.size());
                index_type counter = 0;
                for (auto item : vertexes) {
                    auto aPath = std::get<0>(item);
                    values.push_back(std::make_pair(aPath, counter));
                    counter++;
                }
                std::unordered_map<std::string, index_type> lookupTable(
                    values.begin(), values.end());

                // Prepare the input for our folder hierarchy graph
                std::vector<std::tuple<index_type, index_type>> allEdges;
                allEdges.reserve(edges.size());
                for (auto anEdge : edges) {
                    allEdges.push_back(
                        std::make_tuple(lookupTable[std::get<0>(anEdge)],
                                        lookupTable[std::get<1>(anEdge)]));
                }
                std::sort(allEdges.begin(), allEdges.end());

                // Return vertex information and a folder hierarchy graph.
                return std::make_tuple(vertexes,
                                       SparseGraph<index_type, index_type>(
                                           allEdges, vertexes.size(), true));
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
                            currentPath.string(), aStem,
                            currentPath.extension().string(),
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
        template <typename Container, typename Visitor>
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
         * Search for files in given folders using breath-first-search
         * algorithm.
         *
         * @param searchPaths
         * @param visitor
         *
         * @return
         */
        template <typename Visitor, typename Container>
        size_t bfs_file_search(Container &searchPaths, Visitor &visitor) {            
            size_t counter = 0;
            Container folders(searchPaths.begin(), searchPaths.end());
            while (!folders.empty()) {
                auto aPath = folders.front();
                folders.pop_front();
                visitor.visit(aPath, folders);
            }
            return counter;
        }
    }
}

#endif
