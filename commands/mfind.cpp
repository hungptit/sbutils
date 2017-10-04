// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <array>
#include <fstream>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"

#include "fmt/format.h"

#include "sbutils/Print.hpp"
#include "sbutils/Timer.hpp"
#include "sbutils/Utils.hpp"

#include "sbutils/BasePathVisitor.hpp"
#include "sbutils/MinimalPathVisitor.hpp"
#include "sbutils/PathFilter.hpp"
#include "sbutils/PathSearchAlgorithms.hpp"
#include "sbutils/PathSearchVisitor.hpp"
#include "sbutils/PathVisitor.hpp"
#include "sbutils/SimplePathVisitor.hpp"

template <typename Iterator>
void print(Iterator begin, Iterator end, const bool summary = false) {
    if (summary) {
        fmt::print("Number of found files and folders: {}\n", std::distance(begin, end));
    } else {
        fmt::MemoryWriter writer;
        std::for_each(begin, end, [&writer](auto const &val) { writer << val << "\n"; });
        fmt::print("{}\n", writer.str());
    }
}

int main(int argc, char *argv[]) {
    using namespace boost;
    namespace po = boost::program_options;
    using String = std::string;
    using path = boost::filesystem::path;
    using Container = std::vector<path>;

    po::options_description desc(
        "Usage: mfind folder_paths [options] [paths]\nFind all files and "
        "folders from the list of given folders.\n\nAllowed options:");

    std::vector<std::string> folders;
    std::vector<std::string> stems;
    std::vector<std::string> extensions;
    std::string pattern;

    // clang-format off
    desc.add_options()
        ("help,h", "Print this help")
        ("verbose,v", "Display searched data.")
		("all,a", "Search all files including files in some special folders such as .git.")
		("report", "Only report statistical information")
        ("folders,f", po::value<std::vector<std::string>>(&folders), "Search folders.")
        ("file-stems,s", po::value<std::vector<std::string>>(&stems), "File stems.")
        ("extensions,e", po::value<std::vector<std::string>>(&extensions), "File extensions.")
        ("pattern,p", po::value<std::string>(&pattern), "A search pattern.");
    // clang-format on

    po::positional_options_description p;
    p.add("folders", -1);
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc;
        return EXIT_SUCCESS;
    }

    auto verbose = vm.count("verbose");
    sbutils::ElapsedTime<sbutils::MILLISECOND> timer("Total time: ", verbose);

    std::string jsonFile;
    if (vm.count("toJSON")) {
        jsonFile = vm["toJSON"].as<std::string>();
    }

    using path = boost::filesystem::path;

    boost::system::error_code errcode;

    // If folders is empty then we will
    if (folders.empty()) folders.emplace_back("./");
    Container searchPaths;
    std::for_each(folders.cbegin(), folders.cend(),
                  [&searchPaths](auto item) { searchPaths.emplace_back(path(item)); });

    // Search for files in the given folders.

    using BaseVisitor = typename sbutils::BasePathVisitor<String, Container>;

    if (vm.count("all")) {
        sbutils::MinimalPathVisitor<BaseVisitor> visitor;
        sbutils::dfs_file_search(searchPaths, visitor);
        print(visitor.Paths.cbegin(), visitor.Paths.cend(), vm.count("report"));
    } else {
        if (extensions.empty() && stems.empty()) {
            if (pattern.empty()) {
                // Will skip .git folder.
                sbutils::SimplePathVisitor<BaseVisitor, sbutils::FolderPolicy> visitor;
                sbutils::dfs_file_search(searchPaths, visitor);
                print(visitor.Paths.cbegin(), visitor.Paths.cend(), vm.count("report"));
            } else {
                using NullFilter = sbutils::NullPolicy;
                sbutils::PathSearchVisitor<BaseVisitor, NullFilter, NullFilter> visitor(
                    NullFilter(), NullFilter(), std::move(pattern));
                sbutils::dfs_file_search(searchPaths, visitor);
                print(visitor.Paths.cbegin(), visitor.Paths.cend(), vm.count("report"));
            }

        } else {
            using container_type = decltype(extensions);
            using string_type = std::string;

            sbutils::NormalPolicy<string_type, container_type> fileFilter(
                std::move(stems), std::move(extensions));
            sbutils::FolderPolicy folderFilter;
            sbutils::PathSearchVisitor<BaseVisitor, decltype(folderFilter),
                                       decltype(fileFilter)>
                visitor(folderFilter, fileFilter, std::move(pattern));
            sbutils::dfs_file_search(searchPaths, visitor);
            print(visitor.Paths.cbegin(), visitor.Paths.cend(), vm.count("report"));
        }
    }

    return 0;
}
