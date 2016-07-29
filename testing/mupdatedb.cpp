#include "boost/program_options.hpp"
#include <array>
#include <fstream>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

#include "utils/FileSearch.hpp"
#include "utils/FileUtils.hpp"
#include "utils/LevelDBIO.hpp"
#include "utils/Resources.hpp"
#include "utils/Timer.hpp"

int main(int argc, char *argv[]) {
    using namespace boost;
    using path = boost::filesystem::path;
    
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    std::string dataFile;
    std::string cfgFile;
    
    // clang-format off
    desc.add_options()
        ("help,h", "Print this help")
        ("verbose,v", "Display verbose information.")
        ("folders,f", po::value<std::vector<std::string>>(), "Search folders.")
        ("config,c", po::value<std::string>(&cfgFile)->default_value(".mupdatedb.cfg"), "Search configuratiion.")
        ("database,d", po::value<std::string>(&dataFile)->default_value(".database"), "File database.");
    // clang-format on

    po::positional_options_description p;
    p.add("folders", -1);
    po::variables_map vm;
    po::store(
        po::command_line_parser(argc, argv).options(desc).positional(p).run(),
        vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc;
        return 0;
    }

    bool verbose = vm.count("verbose");
    
    std::vector<path> folders;

    if (vm.count("folders")) {
        auto list = vm["folders"].as<std::vector<std::string>>();
        for (auto item : list) {
            folders.emplace_back(path(utils::normalize_path(item)));
        }
    } else {
        folders.emplace_back(boost::filesystem::current_path());
    }


    // Display input parameters if verbose is true
    if (verbose) {
        fmt::print("verbose: {}\n", verbose);
        fmt::print("config: {}\n", cfgFile);
        fmt::print("database: {}\n", dataFile);
        auto printObj = [](auto const &item) {fmt::print("{}\n", item.string());};
        fmt::print("Search folders: \n");
        std::for_each(folders.cbegin(), folders.cend(), printObj);
    }
    
    // Build file information database
    using FileVisitor =
        utils::filesystem::Visitor<decltype(folders),
                                   utils::filesystem::NormalPolicy>;
    utils::ElapsedTime<utils::MILLISECOND> timer("Total time: ");
    FileVisitor visitor;
    utils::filesystem::dfs_file_search(folders, visitor);

    // Write to database
    if (verbose) {
        visitor.print();
    }
    
    auto results = visitor.getFolderHierarchy();
    visitor.print();
    // auto vertexes = std::get<0>(results);
    // auto g = std::get<1>(results);
    // std::vector<std::string> vids;
    // vids.reserve(vertexes.size());
    // size_t counter = 0;
    // for (auto item : vertexes) {
    //     vids.push_back(std::get<0>(item));
    //     counter += std::get<1>(item).size();
    // }

    // // Build file information database
    // utils::ElapsedTime<utils::MILLISECOND> timer1("Serialization time: ");
    // utils::Writer writer(dataFile);
    // std::ostringstream os;

    // {
    //     utils::DefaultOArchive output(os);
    //     utils::save_sparse_graph(output, g, vids);
    //     writer.write(utils::Resources::GraphKey, os.str());
    // }

    // // Write out all vertex data
    // size_t index = 0;
    // for (auto item : vertexes) {
    //     utils::DefaultOArchive output(os);
    //     os.str(std::string()); // Reset a string stream
    //     auto data = std::get<1>(item);
    //     auto aKey = utils::to_fixed_string(9, index);
    //     utils::save(output, aKey, data);
    //     writer.write(aKey, os.str());
    //     index++;
    // }

    // // Write all file information
    // {
    //     std::vector<utils::FileInfo> allFiles;
    //     allFiles.reserve(counter);
    //     for (auto item : vertexes) {
    //         auto results = std::get<1>(item);
    //         std::move(results.begin(), results.end(),
    //                   std::back_inserter(allFiles));
    //     }

    //     utils::DefaultOArchive output(os);
    //     os.str(std::string()); // Reset a string stream
    //     auto aKey = utils::Resources::AllFileKey;
    //     utils::save(output, aKey, allFiles);
    //     writer.write(aKey, os.str());
    // }

    // Return
    return 0;
}
