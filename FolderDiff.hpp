#ifndef FolderDiff_hpp
#define FolderDiff_hpp

#define BOOST_THREAD_VERSION 4
#include "boost/config.hpp"
#include "boost/thread.hpp"
#include "boost/thread/future.hpp"
#include "boost/unordered_map.hpp"
#include "boost/unordered_set.hpp"

#include <functional>
#include <vector>

#include "DataStructures.hpp"
#include "FileSearch.hpp"
#include "FileUtils.hpp"
#include "RocksDB.hpp"
#include "Timer.hpp"
#include "graph/SparseGraph.hpp"
#include "Utils.hpp"

#include <algorithm>
#include <memory>
#include <string>
#include <utility>

namespace utils {
    // TODO: Use TBB to speed up this function.

    template <typename Container>
    Container read_baseline(const std::string &database,
                            const std::vector<std::string> &folders, bool verbose = false) {
        using IArchive = utils::DefaultIArchive;
        Container allFiles;

        utils::ElapsedTime<utils::MILLISECOND> t("Read baseline: ");

        // Open the database
        std::unique_ptr<rocksdb::DB> db(utils::open(database));
        rocksdb::Status s;

        if (folders.empty()) {
            std::string value;
            rocksdb::Status s =
                db->Get(rocksdb::ReadOptions(), utils::Resources::AllFileKey, &value);
            assert(s.ok());
            std::istringstream is(value);
            IArchive input(is);
            input(allFiles);
            if (verbose) {
                fmt::print("Number of files: {0}\n", allFiles.size());
            }
        } else {
            // utils::ElapsedTime<utils::MILLISECOND> t("Deserialization
            // time:
            // ");
            using index_type = int;
            using edge_type = graph::BasicEdgeData<index_type>;
            using Graph = graph::SparseGraph<index_type, edge_type>;
            using vertex_container = typename Graph::vertex_container;
            using edge_container = typename Graph::edge_container;
            std::string value;
            std::vector<std::string> vids;

            // TODO: Parallelize read vertex ids and read graph info.

            // Read vertex ids
            {
                rocksdb::Status s =
                    db->Get(rocksdb::ReadOptions(), utils::Resources::VIDKey, &value);
                assert(s.ok());
                std::istringstream is(value);
                IArchive input(is);
                input(vids);
            }

            // Now find indexes for given folders using O(n) algorithm. Below
            // code block assume that folders is sorted.
            std::vector<index_type> indexes;
            for (auto const &item : folders) {
                const std::string aKey = utils::normalize_path(item);
                auto it = std::lower_bound(vids.begin(), vids.end(), aKey);
                if (*it == aKey) {
                    indexes.push_back(std::distance(vids.begin(), it));
                } else {
                    fmt::print("Could not find key {} in database\n", aKey);
                }
            }

            // Read graph info
            Graph g;
            {
                value.clear();
                rocksdb::Status s =
                    db->Get(rocksdb::ReadOptions(), utils::Resources::GraphKey, &value);
                assert(s.ok());
                std::istringstream is(value);
                IArchive input(is);
                input(g);
            }

            if (verbose) {
                fmt::print("Number of vertexes: {0}\n", g.numberOfVertexes());
            }

            assert(vids.size() == g.numberOfVertexes());

            std::vector<index_type> allVids =
                graph::dfs_preordering<std::vector<index_type>>(g, indexes);

            // Now read all keys and create a list of edited file data
            // bases.
            {
                std::sort(allVids.begin(), allVids.end());
                const auto readOpts = rocksdb::ReadOptions();
                for (auto const &index : allVids) {
                    const std::string aKey = utils::to_fixed_string(9, index);
                    std::string value;
                    s = db->Get(readOpts, aKey, &value);
                    assert(s.ok());

                    std::istringstream is(value);
                    Vertex<index_type> aVertex;
                    {
                        IArchive input(is);
                        input(aVertex);
                    }
                    std::move(aVertex.Files.begin(), aVertex.Files.end(),
                              std::back_inserter(allFiles));
                }
            }
        }

        return allFiles;
    }

    /**
     * This methods will return a tuple which has
     *     1. Items in first and not in second.
     *     2. Items in both first and second but they are different.
     *     3. Items which are in second and not in first.
     * The first item of each data element is a using string which is a full
     * file name.
     */
    template <typename Container>
    std::tuple<Container, Container, Container>
    diff(const Container &&first, const Container &&second, bool verbose = false) {

        Container modifiedFiles;
        Container results;
        Container newFiles;
        Container deletedFiles;
        
        // Create a lookup table
        using value_type = typename Container::value_type;

        // TODO: Speedup this line using a better hash table.
        std::unordered_set<FileInfo> dict(second.begin(), second.end());

        if (verbose) {
            std::cout << "---- Dictionary sizes: " << dict.size() << " \n";
        }

        // Remove items that belong to both. We assume that the number of
        // modified files is significantly less than the total number of files.
        auto getDiff = [&dict, &results](const auto &item) {
            auto const pos = dict.find(item);
            if (pos != dict.end()) {
                dict.erase(pos);
            } else {
                results.emplace_back(item);
            }
        };

        // TODO: Speed up this loop
        std::for_each(first.begin(), first.end(), getDiff);

        // Get modified and deleted items.
        boost::unordered_map<std::string, value_type> map;
        map.reserve(dict.size());
        for (auto item : dict) {
            map.emplace(std::make_pair(item.Path, item));
        }

        // Note: Modified files are files that are also in the dictionary,
        // however, they have a different size and permission.
        if (!map.empty()) {
            for (auto item : results) {
                const auto pos = map.find(item.Path);
                if (pos != map.end()) {
                    auto dictItem = pos->second;
                    bool isOK = (item.Size == dictItem.Size) &&
                                (item.Permissions == dictItem.Permissions);
                    if (!isOK) {
                        modifiedFiles.emplace_back(item);
                    }
                    map.erase(item.Path);
                } else {
                    newFiles.emplace_back(item);
                }
            }
        } else {
            newFiles = std::move(results);
        }

        // Get deleted items
        std::for_each(map.begin(), map.end(), [&deletedFiles](auto const &item) {
            deletedFiles.emplace_back(item.second);
        });

        // Return
        return std::make_tuple(modifiedFiles, newFiles, deletedFiles);
    }

    auto diffFolders(const std::string &dataFile, const std::vector<std::string> &folders,
                     bool verbose) {
        // Search for files in the given folders.
        using path = boost::filesystem::path;
        using PathContainer = std::vector<path>;
        using Container = std::vector<utils::FileInfo>;

        utils::filesystem::SimpleVisitor<PathContainer, utils::filesystem::NormalPolicy>
            visitor;
        PathContainer searchFolders;
        for (auto item : folders) {
            searchFolders.emplace_back(path(item));
        }

        // We do real work here
        auto searchObj = [&searchFolders, &visitor]() {
            utils::filesystem::dfs_file_search(searchFolders, visitor);
        };
        auto readObj = [dataFile, &folders, verbose]() {
            return utils::read_baseline<Container>(dataFile, folders, verbose);
        };

        boost::future<Container> readThread = boost::async(readObj);
        boost::future<void> findThread = boost::async(searchObj);

        readThread.wait();
        findThread.wait();
        Container baseline = readThread.get();
        findThread.get();

        // Return the differences between baseline and current state.
        Container results = visitor.getResults();
        if (verbose) {
            fmt::print("Number of files: {}\n", results.size());
            fmt::print("Number of files in the baseline: {}\n", baseline.size());
        }

        return utils::diff(std::move(baseline), std::move(results));
    }
}
#endif
