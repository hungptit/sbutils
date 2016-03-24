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
#include "utils/SparseGraph.hpp"
#include "utils/Timer.hpp"
#include "utils/Utils.hpp"
#include "utils/Serialization.hpp"

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
        dataFile = (boost::filesystem::path(utils::FileDatabaseInfo::Database))
                       .string();
    }

    // Search for files in a given folders.
    using namespace utils;
    using Container = std::vector<boost::filesystem::path>;
    utils::ElapsedTime<utils::MILLISECOND> timer;
    // Visitor<Container, DonotFilter> visitor;
    Visitor<Container, NormalFilter> visitor;
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

    // // Generate a tree picture and view it using xdot.
    // std::string dotFile("fg.dot");
    // utils::gendot(g, vertexIDs, dotFile);
    // utils::viewdot(dotFile);

    {
      // Build file information database
      utils::Writer writer(dataFile);

      // Will need to write in batch mode.

      // Write out vertex IDs
      {
        std::ostringstream os;
        utils::save<utils::OArchive, decltype(vertexIDs)>(vertexIDs, os);
        const auto value = os.str();
        writer.write("vertexes", value);
      }

      // Write out graph information
      {
        std::ostringstream os;
        utils::save<utils::OArchive, decltype(vertexIDs)>(vertexIDs, os);
        writer.write("vertex_ids", os.str());
        //fmt::print("{}\n", os.str());
        os.str(std::string());

        auto v = g.getVertexes();
        utils::save<utils::OArchive, decltype(v)>(v, os);
        writer.write("vertexes", os.str());
        //fmt::print("{}\n", os.str());
        os.str(std::string());
        
        auto e = g.getVertexes();
        utils::save<utils::OArchive, decltype(e)>(e, os);
        writer.write("edges", os.str());
        //        fmt::print("{}\n", os.str());
        os.str(std::string());
      }

      // Write out all vertex data
      size_t counter = 0;
      for (auto item : vertexes) {
        std::ostringstream os;
        auto data = std::get<1>(item);
        utils::save<utils::OArchive, decltype(data)>(data, os);
        writer.write(std::to_string(counter), os.str());
        counter ++;
      }
    }

    // {
    //   utils::Reader reader(dataFile);
    //   {
    //     std::string aKey("vertex_ids");
    //     std::istringstream is(reader.read(aKey));
    //     std::vector<std::string> vertex_ids;
    //     utils::load<utils::IArchive, decltype(vertex_ids)>(vertex_ids, is);
    //     fmt::print("Saved vertex ids:\n");
    //     for (auto item : vertex_ids) {
    //       fmt::print("{}\n", item);
    //     }
    //   }
    // }
    
    return 0;
}
