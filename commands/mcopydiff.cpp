// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <array>
#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"

#include "fmt/format.h"

#include "sbutils/FolderDiff.hpp"
#include "sbutils/Timer.hpp"
#include "sbutils/UtilsTBB.hpp"

#include "tbb/task_scheduler_init.h"

int main(int argc, char *argv[]) {
    using namespace boost;
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");

    std::vector<std::string> srcPaths;
    bool verbose = false;
    std::string database;
    std::vector<std::string> dstPaths;
    unsigned int numberOfThreads;

    // clang-format off
    desc.add_options()
        ("help,h", "Print this help")
        ("verbose,v", "Display more information.")
		("max-threads", po::value<unsigned int>(&numberOfThreads)->default_value(2), "Specify the maximum number of used threads.")
        ("src_dir,s", po::value<std::vector<std::string>>(&srcPaths), "Source folder.")
        ("dst_dir,d", po::value<std::vector<std::string>>(&dstPaths), "Destination sandbox.")
        ("database,b", po::value<std::string>(&database)->default_value(sbutils::Resources::Database), "File database.");
    // clang-format on

    po::positional_options_description p;
    p.add("src_dir", -1);
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc;
        fmt::print("Examples:\n\tmcopydiff -s src_folder -d dest_folder\n");
        return 0;
    }

    if (vm.count("verbose")) {
        verbose = true;
    }

    std::vector<std::string> srcDir, dstDir;
    std::for_each(srcPaths.begin(), srcPaths.end(), [&srcDir](const std::string &aPath) {
        srcDir.emplace_back(sbutils::normalize_path(aPath));
    });

    std::for_each(dstPaths.begin(), dstPaths.end(), [&dstDir](const std::string &aPath) {
        dstDir.emplace_back(sbutils::normalize_path(aPath));
    });

    database = sbutils::normalize_path(database);

    if (verbose) {
        std::cout << "Database: " << database << std::endl;
    }

    {
        tbb::task_scheduler_init task_scheduler(numberOfThreads);
        std::vector<sbutils::FileInfo> allEditedFiles, allNewFiles, allDeletedFiles;
        std::tie(allEditedFiles, allDeletedFiles, allNewFiles) =
            sbutils::diffFolders_tbb(database, srcDir, verbose);

        // We will copy new files and edited files to the destiation
        // folder. We also remove all deleted files in the destination
        // folder.
        sbutils::ElapsedTime<sbutils::SECOND> e("Copy files: ", verbose);

        auto runObj = [&allEditedFiles, &allNewFiles, &allDeletedFiles,
                       verbose](const std::string &aDstDir) {
            createParentFolders(aDstDir, allEditedFiles, verbose);
            createParentFolders(aDstDir, allNewFiles, verbose);

            std::tuple<size_t, size_t> results1, results2;
            size_t results3;

            auto copyEditedFileObj = [&allEditedFiles, &aDstDir, verbose, &results1]() {
                results1 = sbutils::copyFiles_tbb(allEditedFiles, aDstDir, verbose);
            };

            auto copyNewFileObj = [&allNewFiles, &aDstDir, verbose, &results2]() {
                results2 = sbutils::copyFiles_tbb(allNewFiles, aDstDir, verbose);
            };

            auto deleteFileObj = [&allDeletedFiles, &aDstDir, verbose, &results3]() {
                results3 = sbutils::deleteFiles(allDeletedFiles, aDstDir, verbose);
            };

            tbb::parallel_invoke(copyEditedFileObj, copyNewFileObj, deleteFileObj);

            fmt::print("==== Summary for {} ====\n", aDstDir);
            fmt::print("\tCopied {0} modified files ({1} bytes)\n", std::get<0>(results1),
                       std::get<1>(results1));
            fmt::print("\tCopied {0} new files ({1} bytes)\n", std::get<0>(results2),
                       std::get<1>(results2));
            fmt::print("\tDelete {0} files\n", results3);
        };

        std::for_each(dstDir.begin(), dstDir.end(), runObj);
    }
}
