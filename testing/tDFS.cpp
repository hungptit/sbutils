#include <array>
#include <fstream>
#include <iostream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"

#include "cppformat/format.h"

#include "utils/FileSearch.hpp"
#include "utils/LevelDBIO.hpp"
#include "utils/Resources.hpp"
#include "utils/Resources.hpp"
#include "utils/Serialization.hpp"
#include "utils/SparseGraph.hpp"
#include "utils/Timer.hpp"
#include "utils/Utils.hpp"

int main(int argc, char *argv[]) {
    using namespace boost;
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");

    // clang-format off
    desc.add_options()
        ("help,h", "Print this help")
        ("verbose,v", "Display searched data.")
        ("folders,f", po::value<std::vector<std::string>>(), "Search folders.")
        ("database,d", po::value<std::string>(), "File database.");
    // clang-format on

    po::positional_options_description p;
    p.add("folders", -1);
    po::variables_map vm;
    po::store(
        po::command_line_parser(argc, argv).options(desc).positional(p).run(),
        vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << "Usage: buildFileDatabase [options]\n";
        std::cout << desc;
        return 0;
    }

    bool verbose = false;
    if (vm.count("verbose")) {
        verbose = true;
    }

    std::vector<std::string> folders;

    if (vm.count("folders")) {
        folders = vm["folders"].as<std::vector<std::string>>();
    } else {
        folders.emplace_back(boost::filesystem::current_path().string());
    }

    std::string dataFile;
    if (vm.count("database")) {
        dataFile = vm["database"].as<std::string>();
    } else {
        dataFile = (boost::filesystem::path(utils::Resources::Database))
                       .string();
    }

    // Search for files in a given folders.
    using namespace utils;
    using Container = std::vector<boost::filesystem::path>;
    utils::ElapsedTime<utils::MILLISECOND> timer;
    // Visitor<Container, DonotFilter> visitor;
    Visitor<Container, NormalPolicy> visitor;
    Container searchFolders;
    for (auto val : folders) {
        searchFolders.emplace_back(val);
    }

    // Travel the file structure tree using DFS algorithm
    dfs_file_search<decltype(visitor), decltype(searchFolders)>(searchFolders,
                                                                visitor);
    visitor.print();
    auto results = visitor.compact();
    auto vertexes = std::get<0>(results);
    auto g = std::get<1>(results);

    // Get all vertex IDs
    std::vector<std::string> vertexIDs;
    vertexIDs.reserve(vertexes.size());
    for (auto item : vertexes) {
        vertexIDs.push_back(std::get<0>(item));
    }

    // // Display the graph information
    // fmt::MemoryWriter writer;
    // size_t counter = 0;
    // for (auto item : vertexes) {
    //   writer << counter << " : " << std::get<0>(item) << "\n";
    //   auto aPath = std::get<0>(item);
    //   counter++;
    // }
    // fmt::print("== Vertexes ==\n{}", writer.str());
    // utils::graph_info(g);

    // Generate a tree picture and view it using xdot.
    // std::string dotFile("fg.dot");
    // utils::gendot(g, vertexIDs, dotFile);
    // utils::viewdot(dotFile);

    
    // using IArchive = cereal::JSONInputArchive;
    // using OArchive = cereal::JSONOutputArchive;

    // using IArchive = cereal::BinaryInputArchive;
    // using OArchive = cereal::BinaryOutputArchive;

    using IArchive = cereal::XMLInputArchive;
    using OArchive = cereal::XMLOutputArchive;

    {
        // Build file information database
        utils::Writer writer(dataFile);

        // Write out the graph information
        std::ostringstream os;
        OArchive output(os);
        utils::save_sparse_graph(output, g, vertexIDs);
        writer.write(utils::Resources::GraphKey, os.str());
        fmt::print("{}\n", os.str());

        // Write out all vertex data
        size_t counter = 0;
        for (auto item : vertexes) {
            os.str(std::string()); // Reset a string stream
            auto data = std::get<1>(item);
            auto aKey = std::to_string(counter);
            output(cereal::make_nvp(aKey, os.str()));
            fmt::print("{}\n", os.str());
            counter++;
        }
    }

    // {
    //     utils::Reader reader(dataFile);
    //     using Index = int;
    //     std::istringstream is(reader.read(utils::Resources::GraphKey));
    //     std::vector<std::string> vids;
    //     std::vector<Index> v;
    //     std::vector<Index> e;
    //     IArchive input(is);
    //     input(vids, v, e);
    //     fmt::print("Saved vertex ids:\n");
    //     utils::SparseGraph<int> g(v, e, true);
    //     for (auto item : vids) {
    //         fmt::print("{}\n", item);
    //     }
    //     utils::graph_info(g);
    // }

    return 0;
}
