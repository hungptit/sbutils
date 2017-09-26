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

#include "graph/DataStructures.hpp"

namespace sbutils {
    // A visitor class for building file information database.
    template <typename String, typename PathContainer, typename Filter> class FileDBVisitor {
      public:
        using path = boost::filesystem::path;
        using directory_iterator = boost::filesystem::directory_iterator;
        using path_container = std::vector<path>;
        using index_type = unsigned int;
        using edge_type = graph::basic_edge_data<index_type>;
        using vertex_type = index_type;

        void visit(const path &aPath, PathContainer &stack) {
            namespace fs = boost::filesystem;
            directory_iterator endIter;
            boost::system::error_code errcode, no_error;

            // Return early if we cannot construct the directory iterator.
            directory_iterator dirIter(aPath, errcode);
            if (errcode != no_error) {
                return;
            }

            const index_type parentIdx = LookupTabble[aPath];
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
                    // TODO: Need to find out the best way to handle symlink.
                    break;
                case boost::filesystem::directory_file:
                    // We need to filter unwanted folders such as ".git".
                    if (!CustomFilter.isValidStem(aStem) ||
                        !CustomFilter.isValidExt(anExtension)) {
                        break;
                    }

                case boost::filesystem::regular_file:
                    const index_type idx = Paths.size();

                    // Update vertex information
                    Paths.emplace_back(currentPath.string());
                    Permissions.push_back(status.permissions());
                    FileSizes.push_back(fs::file_size(currentPath, errcode));
                    LastWriteTimes.push_back(fs::last_write_time(aPath, errcode));

                    // Update graph information
                    Edges.emplace_back(edge_type(parentIdx, idx));
                    stack.emplace_back(currentPath);
                    break;

                default:
                    // Do not know how to handle this case.
                    break;
                }
            }
        }

        template <typename OArchive> void print() {
            fmt::MemoryWriter writer;
            const size_t N = Paths.size();

            for (auto idx = 0; idx < N; ++idx) {
                writer << Paths[idx] << ", " << Permissions[idx] << ", " << FileSizes[idx]
                       << ", " << LastWriteTimes[idx] "\n";
            }

            fmt::print("Summary:\n");
            fmt::print("Number of vertexes: {0}\n", Paths.size());
            fmt::print("Number of edgess: {0}\n", Edges.size());
            fmt::print("Number of files: {0}\n", counter);
        }

      private:
        // Vertex information
        std::vector<String> Paths;
        std::vector<String> Stems;
        std::vector<String> Extensions;
        std::vector<int> Permissions;
        std::vector<uintmax_t> FileSizes;
        std::vector<std::time_t> LastWriteTimes;

        // Dictionary
        std::unordered_map<String, index_type> LookupTabble;

        // Graph information.
        std::vector<edge_type> Edges;
        Filter CustomFilter;
    };

} // namespace sbutils
