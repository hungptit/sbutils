#pragma once

#include <array>
#include <string>
#include <tuple>
#include <vector>

#include "FileUtils.hpp"
#include "FolderDiff.hpp"
#include "UtilsTBB.hpp"

namespace sbutils {
    struct MLocateArgs {
        bool Verbose;
        std::vector<std::string> Folders;
        std::vector<std::string> Extensions;
        std::vector<std::string> Stems;
        std::string Pattern;
        std::string Database;
    };

    auto LocateFiles(MLocateArgs &args) {
        using Container = std::vector<sbutils::FileInfo>;
        std::sort(args.Folders.begin(), args.Folders.end());
        auto data =
            sbutils::read_baseline<Container>(args.Database, args.Folders, args.Verbose);
        const sbutils::ExtFilter<std::vector<std::string>> f1(args.Extensions);
        const sbutils::StemFilter<std::vector<std::string>> f2(args.Stems);
        const sbutils::SimpleFilter f3(args.Pattern);
        auto results = (args.Pattern.empty()) ? filter_tbb(data, f1, f2)
                                              : filter_tbb(data, f1, f2, f3);
        return results;
    }
} // namespace sbutils
