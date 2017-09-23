#pragma once

// STL headers
#include <array>
#include <string>
#include <unordered_map>
#include <vector>

#include "DataStructures.hpp"
#include "Timer.hpp"
#include "Utils.hpp"

#include "boost/filesystem.hpp"
#include "fmt/format.h"

#include "tbb/parallel_sort.h"
#include "tbb/tbb.h"

namespace sbutils {
    namespace filesystem {

        struct DoNothingPolicy {
            bool isValidStem(const std::string &) { return true; }
            bool isValidExt(const std::string &) { return true; }
        };

        class NormalPolicy {
          public:
            bool isValidExt(const std::string &anExtension) {
                return std::find(ExcludedExtensions.begin(), ExcludedExtensions.end(),
                                 anExtension) == ExcludedExtensions.end();
            }

            bool isValidStem(const std::string &aStem) {
                return std::find(ExcludedStems.begin(), ExcludedStems.end(), aStem) ==
                       ExcludedStems.end();
            }

          private:
            const std::array<std::string, 2> ExcludedExtensions = {{".git", ".cache"}};
            const std::array<std::string, 2> ExcludedStems = {{"CMakeFiles", "CMakeTmp"}};
        };

        // This visitor class is used to build the file information database.
        // TODO: Need to optimize for space by storing the file index only.  We
        // also need to rename vertex_data to child_ids or children to make it
        // more intuitive Split file path, permission, stem, extension, file
        // size and use index to store edges.  This will save 2x memory at
        // runtime. We also need to resort file path vector later to improve
        // runtime performance.
        template <typename PathContainer, typename Filter> class Visitor {
          public:
            using container_type = std::vector<FileInfo>;
            using path = boost::filesystem::path;
            using directory_iterator = boost::filesystem::directory_iterator;
            using path_container = std::vector<path>;
            using index_type = unsigned int;
            using vertex_type = Vertex<index_type>;

            void visit(const path &aPath, PathContainer &stack) {
                namespace fs = boost::filesystem;
                directory_iterator endIter;
                boost::system::error_code errcode, no_error;

                // Return early if we cannot construct the directory iterator.
                directory_iterator dirIter(aPath, errcode);
                if (errcode != no_error) {
                    return;
                }

                for (; dirIter != endIter; ++dirIter) {
                    auto const currentPath = dirIter->path();
                    auto const status = dirIter->status(errcode);
                    if (errcode != no_error) {
                        continue; // Move on if we cannot get the status of a given path.
                    }
                    auto const ftype = status.type();
                    const auto aStem = currentPath.stem().string();
                    const auto anExtension = currentPath.extension().string();
                    switch (ftype) {
                    case boost::filesystem::symlink_file:
                    // Treat symlink as a regular file.
                    case boost::filesystem::regular_file:
                        vertex_data.emplace_back(FileInfo(
                            status.permissions(), fs::file_size(currentPath, errcode),
                            currentPath.string(), std::move(aStem), std::move(anExtension),
                            fs::last_write_time(aPath, errcode)));
                        break;
                    case boost::filesystem::directory_file:
                        if (CustomFilter.isValidStem(aStem) &&
                            CustomFilter.isValidExt(anExtension)) {
                            Edges.emplace_back(
                                std::make_tuple(aPath.string(), currentPath.string()));
                            stack.emplace_back(currentPath);
                        }
                        break;
                    default:
                        // Do not know how to handle this case.
                        break;
                    }
                }

                // Each vertex will store its path and a list of files at the
                // root level of the current folder.
                Vertexes.emplace_back(vertex_type{aPath.string(), std::move(vertex_data)});
                vertex_data.clear();
            }

            template <typename OArchive> void print() {
                size_t counter = 0;
                std::for_each(Vertexes.begin(), Vertexes.end(),
                              [&counter](auto const &item) { counter += item.Files.size(); });

                std::stringstream output;

                {
                    OArchive oar(output);
                    oar(cereal::make_nvp("All vertexes", Vertexes));
                }

                fmt::print("{}\n", output.str());

                fmt::print("Summary:\n");
                fmt::print("Number of vertexes: {0}\n", Vertexes.size());
                fmt::print("Number of edgess: {0}\n", Edges.size());
                fmt::print("Number of files: {0}\n", counter);
            }

			// TODO: Need to resort the index.
            template <typename index_type> auto getFolderHierarchy() {
                tbb::parallel_sort(
                    Vertexes.begin(), Vertexes.end(),
                    [](auto const &x, auto const &y) { return x.Path < y.Path; });

                // Create a lookup table
                std::unordered_map<std::string, index_type> lookupTable;
                int counter = 0;
                lookupTable.reserve(Vertexes.size());
                auto updateDictObj = [&lookupTable, &counter](auto const &item) {
                    lookupTable.emplace(std::make_pair(item.Path, counter));
                    ++counter;
                };

                std::for_each(Vertexes.begin(), Vertexes.end(), updateDictObj);

                // Prepare the input for our folder hierarchy graph
                using graph_edge_type = graph::BasicEdgeData<index_type>;
                std::vector<graph_edge_type> allEdges;
                allEdges.reserve(Edges.size());
                for (auto const &anEdge : Edges) {
                    allEdges.push_back(graph_edge_type(lookupTable[std::get<0>(anEdge)],
                                                       lookupTable[std::get<1>(anEdge)]));
                }
                tbb::parallel_sort(allEdges.begin(), allEdges.end());
                return FolderHierarchy<index_type>(std::move(Vertexes), std::move(allEdges));
            }

          private:
            // Temporary variable. Should be on top to improve the performance.
            container_type vertex_data;
            Filter CustomFilter;

            // Information about the folder hierarchy.
            using edge_type = std::tuple<std::string, std::string>;
            std::vector<edge_type> Edges;
            std::vector<vertex_type> Vertexes;

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

            container_type getResults() { return std::move(Results); }

            void visit(const path &aPath, PathContainer &folders) {
                namespace fs = boost::filesystem;
                boost::system::error_code errcode;
                boost::system::error_code no_error;
                directory_iterator endIter;

                // Do not let directory_iterator constructure is failed then we
                // won't visit a given folder.
                directory_iterator dirIter(aPath, errcode);
                if (errcode != no_error) {
                    return;
                }

                for (; dirIter != endIter; ++dirIter) {
                    const auto currentPath = dirIter->path();
                    const auto status = dirIter->status(errcode);
                    if (errcode != no_error) {
                        continue; // Move on if we cannot get the status of the current path.
                    }
                    const auto perms = status.permissions();
                    const auto ftype = status.type();
                    const auto aStem = currentPath.stem().string();
                    const auto anExtension = currentPath.extension().string();
                    switch (ftype) {
                    case boost::filesystem::symlink_file: // Treat symbolic link as a regular
                                                          // file.
                    case boost::filesystem::regular_file:
                        Results.emplace_back(FileInfo(
                            perms, fs::file_size(currentPath, errcode), currentPath.string(),
                            std::move(aStem), std::move(anExtension),
                            fs::last_write_time(aPath, errcode)));
                        break;
                    case boost::filesystem::directory_file:
                        if (CustomFilter.isValidStem(aStem) &&
                            CustomFilter.isValidExt(anExtension)) {
                            folders.emplace_back(currentPath);
                        }
                        break;
                    default:
                        // Do not know how to handle this case.
                        break;
                    }
                }
            }

          private:
            Filter CustomFilter;
            std::vector<FileInfo> Results;
        };

        // Search for files which satisfy FolderFilter and FileFilter constraints
        template <typename PathContainer, typename FolderFilter, typename FileFilter>
        class SimpleSearchVisitor {
          public:
            using container_type = std::vector<FileInfo>;
            using path = boost::filesystem::path;
            using directory_iterator = boost::filesystem::directory_iterator;
            using path_container = std::vector<path>;

            std::vector<std::string> getResults() { return std::move(Results); }

            void visit(const path &aPath, PathContainer &folders) {
                namespace fs = boost::filesystem;
                boost::system::error_code errcode;
                boost::system::error_code no_error;
                directory_iterator endIter;

                // Do not let directory_iterator constructure is failed then we
                // won't visit a given folder.
                directory_iterator dirIter(aPath, errcode);
                if (errcode != no_error) {
                    return;
                }

                for (; dirIter != endIter; ++dirIter) {
                    const auto currentPath = dirIter->path();
                    const auto status = dirIter->status(errcode);
                    if (errcode != no_error) {
                        continue; // Move on if we cannot get the status of the current path.
                    }
                    const auto ftype = status.type();
                    const auto aStem = currentPath.stem().string();
                    const auto anExtension = currentPath.extension().string();
                    switch (ftype) {
                    case boost::filesystem::symlink_file: // Treat symbolic link as a regular
                                                          // file.
                    case boost::filesystem::regular_file:
                        if (CustomFileFilter.isValidStem(aStem) &&
                            CustomFileFilter.isValidExt(anExtension)) {
                            Results.emplace_back(currentPath.string());
                        }
                        break;
                    case boost::filesystem::directory_file:
                        if (CustomFolderFilter.isValidStem(aStem) &&
                            CustomFolderFilter.isValidExt(anExtension)) {
                            folders.emplace_back(currentPath);
                        }
                        break;
                    default:
                        // Do not know how to handle this case.
                        break;
                    }
                }
            }

          private:
            FolderFilter CustomFolderFilter;
            FileFilter CustomFileFilter;
            std::vector<std::string> Results;
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
        void dfs_file_search(const Container &searchPaths, Visitor &visitor,
                             bool verbose = false) {
            ElapsedTime<MILLISECOND> timer("Search files: ", verbose);
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
} // namespace sbutils
