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

#include "DataStructures.hpp"
#include "Utils.hpp"

namespace utils {
    namespace filesystem {
        template <typename Container> class ExtFilter {
          public:
            explicit ExtFilter(Container &exts) : Extensions(exts) {}

            bool isValid(FileInfo &info) {
                if (Extensions.empty()) {
                    return true;
                } else {
                    return (std::find(Extensions.begin(), Extensions.end(),
                                      info.Extension) != Extensions.end());
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
                    return (std::find(Stems.begin(), Stems.end(), info.Stem) !=
                            Stems.end());
                }
            }

          private:
            std::vector<std::string> Stems;
        };

        template <typename Iterator, typename Filter1>
        std::vector<utils::FileInfo> filter(Iterator begin, Iterator end,
                                            Filter1 &f1) {
            std::vector<utils::FileInfo> results;
            auto filterObj = [&f1, &results](auto &item) {
                if (f1.isValid(item)) {
                    results.emplace_back(item);
                }
            };
            std::for_each(begin, end, filterObj);
            return results;
        }

        template <typename Iterator, typename Filter1, typename Filter2>
        std::vector<utils::FileInfo> filter(Iterator begin, Iterator end,
                                            Filter1 &f1, Filter2 &f2) {
            std::vector<utils::FileInfo> results;
            auto filterObj = [&f1, &f2, &results](auto &item) {
                if (f1.isValid(item) && f2.isValid(item)) {
                    results.emplace_back(item);
                }
            };
            std::for_each(begin, end, filterObj);
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
            using index_type = int;
            using vertex_type = Vertex<index_type>;
            // using graph_type = graph::SparseGraph<index_type, index_type>;

            void visit(path &aPath, PathContainer &stack) {
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
                        vertex_data.emplace_back(FileInfo(
                            status.permissions(),
                            fs::file_size(currentPath, errcode), currentPathStr,
                            aStem, currentPath.extension().string(),
                            fs::last_write_time(aPath, errcode)));
                    } else if (ftype == boost::filesystem::directory_file) {
                        if (CustomFilter.isValidStem(aStem) &&
                            CustomFilter.isValidExt(anExtension)) {
                            edges.emplace_back(std::make_tuple(
                                aPath.string(), currentPath.string()));
                            stack.emplace_back(currentPath);
                        }
                    } else {
                    }
                }

                // Each vertex will store its path and a list of files at the
                // root
                // level of the current folder.
                vertexes.emplace_back(
                    vertex_type{aPath.string(), std::move(vertex_data)});
                vertex_data.clear();
            }

            template <typename OArchive> void print() {
                size_t counter = 0;
                std::for_each(vertexes.begin(), vertexes.end(),
                              [&counter](auto const &item) {
                                  counter += item.Files.size();
                              });

                std::stringstream output;

                
                
                {
                    OArchive oar(output);
                    oar(cereal::make_nvp("All vertexes", vertexes));
                }

                fmt::print("{}\n", output.str());

                fmt::print("Summary:\n");
                fmt::print("Number of vertexes: {0}\n", vertexes.size());
                fmt::print("Number of edgess: {0}\n", edges.size());
                fmt::print("Number of files: {0}\n", counter);
            }

            // TODO: Use data structures and algorithms provided by graph
            // module.
            auto getFolderHierarchy() {
                using index_type = int;
                std::sort(vertexes.begin(), vertexes.end(),
                          [](auto const &x, auto const &y) {
                              return x.Path < y.Path;
                          });

                // Create a lookup table
                std::unordered_map<std::string, index_type> lookupTable;
                int counter = 0;
                lookupTable.reserve(vertexes.size());
                auto updateDictObj = [&](auto const &item) {
                    lookupTable.emplace(std::make_pair(item.Path, counter));
                    ++counter;
                };

                std::for_each(vertexes.begin(), vertexes.end(), updateDictObj);

                // Prepare the input for our folder hierarchy graph
                using graph_edge_type = graph::BasicEdgeData<index_type>;
                std::vector<graph_edge_type> allEdges;
                allEdges.reserve(edges.size());
                for (auto anEdge : edges) {
                    allEdges.push_back(graph_edge_type(lookupTable[std::get<0>(anEdge)],
                                                 lookupTable[std::get<1>(anEdge)]));
                }
                std::sort(allEdges.begin(), allEdges.end(), graph::Less<index_type, graph_edge_type>());
                return FolderHierarchy<index_type>(std::move(vertexes), std::move(allEdges));
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
                        Results.emplace_back(FileInfo(
                            status.permissions(), fs::file_size(currentPath),
                            currentPath.string(), aStem,
                            currentPath.extension().string(),
                            fs::last_write_time(aPath)));
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
    }
}

#endif
