#include "boost/program_options.hpp"
#include "utils/LevelDBIO.hpp"
#include "utils/Timer.hpp"
#include "utils/Utils.hpp"
#include <array>
#include <fstream>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

#include "cppformat/format.h"

namespace {
    typedef std::tuple<std::string, std::string, std::string, int, std::time_t,
                       uintmax_t>
        FileInfo;

    using path = boost::filesystem::path;


    template <typename PathContainer> class Visitor {
      public:
        using container_type = std::vector<FileInfo>;
        using path = boost::filesystem::path;
        using directory_iterator = boost::filesystem::directory_iterator;
        using path_container = std::vector<path>;

        container_type getResults() {return Results;}

        void visit(path &aPath, PathContainer &folders) {
            namespace fs = boost::filesystem;
            directory_iterator endIter;
            directory_iterator dirIter(aPath);
            for (; dirIter != endIter; ++dirIter) {
                auto currentPath = dirIter->path();
                auto status = dirIter->status();
                auto ftype = status.type();
                std::string currentPathStr = currentPath.string();
                if (ftype == boost::filesystem::regular_file) {
                    vertex_data.emplace_back(std::make_tuple(currentPathStr, currentPath.stem().string(),
                                                             currentPath.extension().string(), status.permissions(),
                                                             fs::last_write_time(aPath), fs::file_size(currentPath)));
                } else if (ftype == boost::filesystem::directory_file) {
                    edges.emplace_back(std::make_tuple(aPath.string(), currentPath.string()));
                    folders.emplace_back(currentPath);
                } else {
                    
                }
            }

            vertexes.emplace_back(aPath.string(), vertex_data);
            vertex_data.clear();
        }

        void print() {
            size_t counter = 0;
            for (auto const & item : vertexes) {
                counter += std::get<1>(item).size();
            }
            fmt::print("Number of vertexes: {}\n", vertexes.size());
            fmt::print("Number of edgess: {}\n", edges.size());
            fmt::print("Number of files: {}\n", counter);
        }

      private:
        container_type Results;
        container_type vertex_data; // Hold a list of all files for a given folders.
        std::vector<std::tuple<std::string, std::string>> edges; // Edge information
        std::vector<std::tuple<std::string, container_type>> vertexes; // Vertex information
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
        size_t counter = 0;
        Container folders(searchPaths.begin(), searchPaths.end());
        while (!folders.empty()) {
            auto aPath = folders.front();
            folders.pop_front();
            visitor.visit(aPath, folders);
        }
        return counter;
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
    
    constexpr int LOOP = 5;
    
    for (int i = 0 ; i < LOOP; i++) 
    {
        using Container = std::vector<boost::filesystem::path>;
        utils::ElapsedTime<utils::MILLISECOND> timer;
        Visitor<Container> visitor;
        Container searchFolders;
        for (auto val : folders) {
            searchFolders.emplace_back(val);
        }

        // Travel the file structure tree using DFS algorithm
        dfs_file_search<decltype(visitor), decltype(searchFolders)>(searchFolders, visitor);
        visitor.print();
    }

    for (int i = 0 ; i < LOOP; i++) 
    {
        using Container = std::deque<boost::filesystem::path>;
        utils::ElapsedTime<utils::MILLISECOND> timer;
        Visitor<Container> visitor;
        Container searchFolders;
        for (auto val : folders) {
            searchFolders.emplace_back(val);
        }

        // Travel the file structure tree using DFS algorithm
        bfs_file_search<decltype(visitor), decltype(searchFolders)>(searchFolders, visitor);
        visitor.print();
    }

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

