#pragma once

#include "PathVisitor.hpp"

namespace sbutils {
    // A visitor class for building file information database.
    template <typename String, typename PathContainer, typename FFilter, typename DFilter>
    class FileDBVisitor {
      public:
        using path = boost::filesystem::path;
        using directory_iterator = boost::filesystem::directory_iterator;
        using path_container = PathContainer;
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

            auto const it = LookupTable.find(aPath.string());
            const bool hasParent = it != LookupTable.end();
            const index_type parentIdx = hasParent ? it->second : 0;

            for (; dirIter != endIter; ++dirIter) {
                auto const &currentPath = dirIter->path();
                auto const &currentPathStr = currentPath.string();
                auto const it = Status.find(currentPathStr);
                if (it != Status.end()) {
                    continue; // The current path has been processed.
                }

                auto const status = dirIter->status(errcode);
                if (errcode != no_error) {
                    continue; // Move on if we cannot get the status of a given path.
                }

                const file_type ftype = status.type();
                const auto aStem = currentPath.stem().string();
                const auto anExtension = currentPath.extension().string();

                if (ftype == boost::filesystem::regular_file) {
                    Status.emplace(currentPathStr);
                    if (hasParent) {
                        Edges.emplace_back(edge_type(parentIdx, Paths.size()));
                    }
                    Paths.emplace_back(currentPath.string());
                    Permissions.push_back(status.permissions());
                    FileSizes.push_back(fs::file_size(currentPath, errcode));
                    LastWriteTimes.push_back(fs::last_write_time(currentPath, errcode));
                    Types.push_back(ftype);
                }

                if (ftype == boost::filesystem::directory_file) {
                    Status.emplace(currentPathStr);
                    if (DirFilter.isValidStem(aStem) &&
                        DirFilter.isValidExt(anExtension)) { // We need to filter unwanted
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
                }
            }
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
        FFilter FileFilter;
		DFilter DirFilter;
        std::unordered_set<String> Status;
		std::unordered_map<String, index_type> LookupTable;

        // Edge information.
        std::vector<edge_type> Edges;
    };
} // namespace sbutils
