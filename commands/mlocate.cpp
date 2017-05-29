#include "boost/program_options.hpp"
#include <array>
#include <fstream>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

#include "sbutils/FileSearch.hpp"
#include "sbutils/FileUtils.hpp"
#include "sbutils/FolderDiff.hpp"
#include "sbutils/Resources.hpp"
#include "sbutils/Timer.hpp"
#include "sbutils/UtilsTBB.hpp"

// #include "tbb/task_scheduler_init.h"

template <typename Container> void print(Container &&results) {
    fmt::MemoryWriter writer;
    // writer << "Search results: \n";
    std::for_each(results.begin(), results.end(),
                  [&writer](auto const &item) { writer << item.Path << "\n"; });
    fmt::print("{}", writer.str());
}

int main(int argc, char *argv[]) {
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    std::string database;
    std::vector<std::string> stems;
    std::vector<std::string> extensions;
    std::vector<std::string> folders;
    std::string pattern;

    // clang-format off
    desc.add_options()
        ("help,h", "Print this help")
        ("verbose,v", "Display searched data.")
        ("keys,k", "List all keys.")
        ("folders,f", po::value<std::vector<std::string>>(&folders), "Search folders.")
        ("stems,s", po::value<std::vector<std::string>>(&stems), "File stems.")
        ("extensions,e", po::value<std::vector<std::string>>(&extensions), "File extensions.")
        ("pattern,p", po::value<std::string>(&pattern), "Search string pattern.")
        ("database,d", po::value<std::string>(&database)->default_value(sbutils::Resources::Database), "File database.");
    // clang-format on

    po::positional_options_description p;
    p.add("pattern", -1);
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << "Usage: mlocate [options]\n";
        std::cout << desc;
        std::cout << "Examples:\n";
        std::cout << "\t mlocate -d .database -s AutoFix\n";
        std::cout << "\t mlocate -s AutoFix\n";
        return 0;
    }

	// Check that the given database is valid
	{
		boost::filesystem::path aPath(database);
		if (!boost::filesystem::exists(database)) {
			throw std::runtime_error("Could not open the file information database: " + database + "\n");
		}
	}
	
    bool verbose = vm.count("verbose");
    sbutils::ElapsedTime<sbutils::MILLISECOND> timer("Total time: ", verbose);

    if (verbose) {
        std::cout << "Database: " << database << std::endl;
    }

    using Container = std::vector<sbutils::FileInfo>;
    std::sort(folders.begin(), folders.end());
    auto data = sbutils::read_baseline<Container>(database, folders, verbose);
    const sbutils::ExtFilter<std::vector<std::string>> f1(extensions);
    const sbutils::StemFilter<std::vector<std::string>> f2(stems);
    const sbutils::SimpleFilter f3(pattern);
    auto results = (pattern.empty()) ? sbutils::filter_tbb(data, f1, f2)
                                     : sbutils::filter_tbb(data, f1, f2, f3);
    print(results);
}
