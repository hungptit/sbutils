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

#include "utils/LevelDBIO.hpp"
#include "utils/Resources.hpp"
#include "utils/SparseGraph.hpp"
#include "utils/Timer.hpp"

namespace {
    typedef std::tuple<std::string, std::string, std::string, int, std::time_t,
                       uintmax_t>
        FileInfo;

    struct DonotFilter {
        bool isValidStem(const std::string &) { return true; }
        bool isValidExt(const std::string &) { return true; }
    };

    class NormalFilter {
      public:
        bool isValidStem(const std::string &anExtension) {
            return std::find(ExcludedExtensions.begin(),
                             ExcludedExtensions.end(),
                             anExtension) == ExcludedExtensions.end();
        }

        bool isValidExt(const std::string &aStem) {
            return std::find(ExcludedStems.begin(), ExcludedStems.end(),
                             aStem) == ExcludedStems.end();
        }

      private:
        const std::array<std::string, 1> ExcludedExtensions = {{".git"}};
        const std::array<std::string, 1> ExcludedStems = {{"CMakeFiles"}};
    };

    class MWFilter {
      public:
        bool isValidStem(const std::string &anExtension) {
            return std::find(ExcludedExtensions.begin(),
                             ExcludedExtensions.end(),
                             anExtension) == ExcludedExtensions.end();
        }

        bool isValidExt(const std::string &aStem) {
            return std::find(ExcludedStems.begin(), ExcludedStems.end(),
                             aStem) == ExcludedStems.end();
        }

      private:
        const std::array<std::string, 2> ExcludedExtensions = {{".sbtools"}};
        const std::array<std::string, 2> ExcludedStems = {{"doc", "doxygen"}};
    };

    template <typename PathContainer, typename Filter> class Visitor {
      public:
        using container_type = std::vector<FileInfo>;
        using path = boost::filesystem::path;
        using directory_iterator = boost::filesystem::directory_iterator;
        using path_container = std::vector<path>;
        using vertex_type = std::tuple<std::string, container_type>;
        using Index = int;

        void visit(path &aPath, PathContainer &folders) {
            namespace fs = boost::filesystem;
            directory_iterator endIter;
            directory_iterator dirIter(aPath);
            for (; dirIter != endIter; ++dirIter) {
                auto currentPath = dirIter->path();
                auto status = dirIter->status();
                auto ftype = status.type();
                std::string currentPathStr = currentPath.string();
                auto aStem = currentPath.stem().string();
                auto anExtension = currentPath.extension().string();
                if (ftype == boost::filesystem::regular_file) {
                    vertex_data.emplace_back(std::make_tuple(
                        currentPathStr, aStem, currentPath.extension().string(),
                        status.permissions(), fs::last_write_time(aPath),
                        fs::file_size(currentPath)));
                } else if (ftype == boost::filesystem::directory_file) {
                    if (CustomFilter.isValidStem(aStem) &&
                        CustomFilter.isValidExt(anExtension)) {
                        edges.emplace_back(std::make_tuple(
                            aPath.string(), currentPath.string()));
                        folders.emplace_back(currentPath);
                    }
                } else {
                }
            }

            vertexes.emplace_back(aPath.string(), vertex_data);
            vertex_data.clear();
        }

        void print() {
            size_t counter = 0;
            for (auto const &item : vertexes) {
                counter += std::get<1>(item).size();
            }
            fmt::print("Number of vertexes: {0}\n", vertexes.size());
            fmt::print("Number of edgess: {0}\n", edges.size());
            fmt::print("Number of files: {0}\n", counter);
        }

        auto compact() {
            using Index = int;

            // Create a lookup table
            std::sort(vertexes.begin(), vertexes.end());
            std::vector<std::pair<std::string, Index>> values;
            values.reserve(vertexes.size());
            Index counter = 0;
            for (auto item : vertexes) {
                auto aPath = std::get<0>(item);
                values.push_back(std::make_pair(aPath, counter));
                counter++;
            }
            std::unordered_map<std::string, Index> lookupTable(values.begin(),
                                                               values.end());

            // Prepare the input for our folder hierarchy graph
            std::vector<std::tuple<Index, Index>> allEdges;
            allEdges.reserve(edges.size());
            for (auto anEdge : edges) {
                allEdges.push_back(
                    std::make_tuple(lookupTable[std::get<0>(anEdge)],
                                    lookupTable[std::get<1>(anEdge)]));
            }
            std::sort(allEdges.begin(), allEdges.end());

            // Return vertex information and a folder hierarchy graph.
            return std::make_tuple(
                vertexes,
                utils::SparseGraph<Index>(allEdges, vertexes.size(), true));
        }

      private:
        // Temporary variable. Should be on top to improve the performance.
        container_type vertex_data;
        Filter CustomFilter;

        // Information about the folder hierarchy.
        using edge_type = std::tuple<std::string, std::string>;
        std::vector<edge_type> edges;
        std::vector<vertex_type> vertexes;

        // Data that will be used to filter search space.
        std::array<std::string, 1> ExcludedExtensions = {{".git"}};
        std::array<std::string, 1> ExcludedStems = {{"CMakeFiles"}};
    };

    /**
     * Search for files in given folders using depth-first-search algorithm.
     *
     * @param searchPaths
     * @param visitor
     *
     * @return
     */
    template <typename Visitor, typename Container>
    size_t dfs_file_search(Container &searchPaths, Visitor &visitor) {
        using path = boost::filesystem::path;
        size_t counter = 0;
        std::vector<path> folders(searchPaths.begin(), searchPaths.end());
        while (!folders.empty()) {
            auto aPath = folders.back();
            folders.pop_back();
            visitor.visit(aPath, folders);
        }
        return counter;
    }

    /**
     * Search for files in given folders using breath-first-search algorithm.
     *
     * @param searchPaths
     * @param visitor
     *
     * @return
     */
    template <typename Visitor, typename Container>
    size_t bfs_file_search(Container &searchPaths, Visitor &visitor) {
        using path = boost::filesystem::path;
        size_t counter = 0;
        Container folders(searchPaths.begin(), searchPaths.end());
        while (!folders.empty()) {
            auto aPath = folders.front();
            folders.pop_front();
            visitor.visit(aPath, folders);
        }
        return counter;
    }

    template <typename Writer> void write(const std::string &dataFile) {
    }
}

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

    // Build file information database
    utils::Writer writer(dataFile);

    // constexpr int LOOP = 5;

    // for (int i = 0; i < LOOP; i++)
    {
        using Container = std::vector<boost::filesystem::path>;
        utils::ElapsedTime<utils::MILLISECOND> timer;
        // Visitor<Container, DonotFilter> visitor;
        Visitor<Container, MWFilter> visitor;
        Container searchFolders;
        for (auto val : folders) {
            searchFolders.emplace_back(val);
        }

        // Travel the file structure tree using DFS algorithm
        dfs_file_search<decltype(visitor), decltype(searchFolders)>(
            searchFolders, visitor);
        visitor.print();
        auto results = visitor.compact();
        auto vertexes = std::get<0>(results);
        auto g = std::get<1>(results);

        // Display the graph information
        // {
        //     std::vector<std::string> vertexIDs;
        //     vertexIDs.reserve(vertexes.size());
        //     fmt::MemoryWriter writer;
        //     size_t counter = 0;
        //     for (auto item : vertexes) {
        //         writer << counter << " : " << std::get<0>(item) << "\n";
        //         auto aPath = std::get<0>(item);
        //         vertexIDs.push_back(aPath);
        //         counter++;
        //     }
        //     fmt::print("== Vertexes ==\n{}", writer.str());
        //     utils::graph_info(g);

        //     // Generate a tree picture and view it using xdot.
        //     std::string dotFile("fg.dot");
        //     utils::gendot(g, vertexIDs, dotFile);
        //     utils::viewdot(dotFile);
        // }
    }

    // for (int i = 0; i < LOOP; i++) {
    //     using Container = std::deque<boost::filesystem::path>;
    //     utils::ElapsedTime<utils::MILLISECOND> timer;
    //     Visitor<Container> visitor;
    //     Container searchFolders;
    //     for (auto val : folders) {
    //         searchFolders.emplace_back(val);
    //     }

    //     // Travel the file structure tree using DFS algorithm
    //     bfs_file_search<decltype(visitor), decltype(searchFolders)>(
    //         searchFolders, visitor);
    //     visitor.print();
    // }

    // for (const auto &aFolder : folders) {
    //     // Search for files
    //     const std::string aPath = utils::getAbslutePath(aFolder);
    //     utils::FileSearchBase<utils::DFSFileSearchBase> fSearch;
    //     fSearch.search(aPath);

    //     // Serialized file information to string
    //     std::ostringstream os;
    //     auto data = fSearch.getData();
    //     utils::save<utils::OArchive, decltype(data)>(data, os);
    //     const auto value = os.str();

    //     // Write searched info to database.
    //     writer.write(aPath, value);

    //     // Display the information if the verbose flag is set.
    //     if (verbose) {
    //         for (auto const &info : data) {
    //             std::cout << info << "\n";
    //         }
    //     }
    // }
    return 0;
}
