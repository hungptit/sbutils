#pragma once

// STL headers
#include <array>
#include <string>
#include <unordered_map>
#include <vector>

// Other headers
#include "PathFilter.hpp"
#include "boost/filesystem.hpp"
#include "fmt/format.h"
#include "graph/DataStructures.hpp"

namespace filesystem {
    enum DirStatus { UNDISCOVERED, VISITED, PROCESSED };

    // A visitor class for building file information database.
    template <typename String, typename PathContainer, typename Filter> class FileDBVisitor {
      public:
        using path = boost::filesystem::path;
        using directory_iterator = boost::filesystem::directory_iterator;
        using path_container = std::vector<path>;
        using index_type = unsigned int;
        using edge_type = graph::BasicEdgeData<index_type>;
        using vertex_type = index_type;
        using file_type = boost::filesystem::file_type;

        // There is a bug in visit method.
        void visit(const path &aPath, PathContainer &stack) {
            namespace fs = boost::filesystem;
            directory_iterator endIter;
            boost::system::error_code errcode, no_error;

            // Return early if we cannot construct the directory iterator.
            directory_iterator dirIter(aPath, errcode);
            if (errcode != no_error) {
                return;
            }

            auto const it = LookupTabble.find(aPath.string());
            const bool hasParent = it != LookupTabble.end();
            const index_type parentIdx = hasParent ? it->second : 0;

            for (; dirIter != endIter; ++dirIter) {
                auto const currentPath = dirIter->path();
                auto const currentPathStr = currentPath.string();
                auto const it = Status.find(currentPathStr);
                if (it != Status.end()) {
                    continue; // The current path has been processed.
                }

                // If we have already visit this path then move on.
                if (LookupTabble.find(currentPathStr) != LookupTabble.end()) {
                    continue;
                }

                auto const status = dirIter->status(errcode);
                if (errcode != no_error) {
                    continue; // Move on if we cannot get the status of a given path.
                }

                const file_type ftype = status.type();
                const auto aStem = currentPath.stem().string();
                const auto anExtension = currentPath.extension().string();

                switch (ftype) {
                case boost::filesystem::symlink_file:
                    // TODO: Need to find out the best way to handle symlink.
                    break;
                case boost::filesystem::regular_file:
                    Status.emplace(std::make_pair(currentPathStr, PROCESSED));
                    if (hasParent) {
                        Edges.emplace_back(edge_type(parentIdx, Paths.size()));
                    }
                    Paths.emplace_back(currentPath.string());
                    Permissions.push_back(status.permissions());
                    FileSizes.push_back(fs::file_size(currentPath, errcode));
                    LastWriteTimes.push_back(fs::last_write_time(currentPath, errcode));
                    Types.push_back(ftype);
                    break;
                case boost::filesystem::directory_file:
                    Status.emplace(std::make_pair(currentPathStr, PROCESSED));
                    if (CustomFilter.isValidStem(aStem) &&
                        CustomFilter.isValidExt(anExtension)) { // We need to filter unwanted
                                                                // folders such as ".git".
                        if (hasParent) {
                            Edges.emplace_back(edge_type(parentIdx, Paths.size()));
                        }
                        Paths.emplace_back(currentPath.string());
                        Permissions.push_back(status.permissions());

                        // We will compute the the size of folders later.
                        FileSizes.push_back(0);

                        LastWriteTimes.push_back(fs::last_write_time(currentPath, errcode));
                        Types.push_back(ftype);
                        stack.emplace_back(currentPath);
                    }
                    break;

                default:
                    // Do not know how to handle this case.
                    break;
                }
            }
        }

        void print() {
            fmt::MemoryWriter writer;
            const size_t N = Paths.size();

            for (auto idx = 0; idx < N; ++idx) {
                writer << Paths[idx] << ", " << Permissions[idx] << ", " << FileSizes[idx]
                       << ", " << LastWriteTimes[idx] << "," << Types[idx] << "\n";
            }

            fmt::print("Summary:\n");
            fmt::print("Number of vertexes: {0}\n", Paths.size());
            fmt::print("Number of edgess: {0}\n", Edges.size());
            fmt::print(writer.str());
        }

      public:
        // Vertex information
        std::vector<String> Paths;
        std::vector<String> Stems;
        std::vector<String> Extensions;
        std::vector<int> Permissions;
        std::vector<uintmax_t> FileSizes;
        std::vector<std::time_t> LastWriteTimes;
        std::vector<file_type> Types;

      private:
        Filter CustomFilter;

        // Dictionary
        std::unordered_map<String, index_type> LookupTabble;
        std::unordered_map<String, DirStatus> Status;

        // Edge information.
        std::vector<edge_type> Edges;
    };

} // namespace filesystem
