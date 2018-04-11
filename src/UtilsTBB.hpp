#pragma once

#include <iomanip>
#include <iostream>
#include <string>
#include <tuple>
#include <utility>
#include <type_traits>

#include "DataStructures.hpp"
#include "utils/timer.hpp"
#include "Utils.hpp"
#include "boost/algorithm/searching/knuth_morris_pratt.hpp"

#include "tbb/parallel_invoke.h"
#include "tbb/tbb.h"

namespace sbutils {
    template <typename Container, typename FirstConstraint, typename... Constraints>
    auto filter_tbb(Container &&data, FirstConstraint &&f1, Constraints &&... fs) {
        // utils::ElapsedTime<MILLISECOND> t1("Filtering files: ");
		using container_type = typename std::decay<Container>::type;
		using output_type = typename container_type::value_type;
        tbb::concurrent_vector<output_type> results;

        auto filterObj = [&f1, &fs..., &results, &data](const int idx) {
            auto const &item = data[idx];
            if (isValid(item, f1, std::forward<Constraints>(fs)...)) {
                results.push_back(item);
            }
        };

        int size = static_cast<int>(data.size());

		// TODO: Need to split data into reasonable size blocks.
        tbb::parallel_for(0, size, 1, filterObj);
        return results;
    }

    class CopyFiles {
      public:
        using path = boost::filesystem::path;
        const path DstDir;
        const std::vector<FileInfo> *Files;
        bool Verbose;
        size_t NumberOfCopiedFiles = 0;
        size_t NumberOfCopiedBytes = 0;

        CopyFiles(const path &dstDir, const std::vector<FileInfo> *files, bool verbose)
            : DstDir(dstDir), Files(files), Verbose(verbose), NumberOfCopiedFiles(0),
              NumberOfCopiedBytes(0) {}

        void operator()(const tbb::blocked_range<size_t> &r) {
            const size_t begin = r.begin();
            const size_t end = r.end();
            for (size_t idx = begin; idx != end; ++idx) {
                const FileInfo info = (*Files)[idx];
                if (copyAFile(DstDir, info, Verbose)) {
                    NumberOfCopiedFiles++;
                    NumberOfCopiedBytes += info.Size;
                }
            }
        }

        CopyFiles(const CopyFiles &rhs, tbb::split)
            : DstDir(rhs.DstDir), Files(rhs.Files), Verbose(rhs.Verbose) {}

        void join(const CopyFiles &rhs) {
            NumberOfCopiedFiles += rhs.NumberOfCopiedFiles;
            NumberOfCopiedBytes += rhs.NumberOfCopiedBytes;
        }
    };

    auto copyFiles_tbb(const std::vector<FileInfo> &files,
                       const boost::filesystem::path &dstDir, bool verbose = false) {

        CopyFiles copyObj(dstDir, &files, verbose);
        tbb::parallel_reduce(tbb::blocked_range<size_t>(0, files.size()), copyObj);
        return std::make_tuple(copyObj.NumberOfCopiedFiles, copyObj.NumberOfCopiedBytes);
    }
}
