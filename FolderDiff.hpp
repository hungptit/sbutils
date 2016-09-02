#ifndef FolderDiff_hpp
#define FolderDiff_hpp

#include <functional>
#include <future>
#include <thread>
#include <vector>

#include "DataStructures.hpp"
#include "FileSearch.hpp"
#include "FileUtils.hpp"
#include "RocksDB.hpp"
#include "Timer.hpp"
#include "Utils.hpp"
#include "graph/SparseGraph.hpp"

#define BOOST_THREAD_VERSION 4
#include "boost/config.hpp"
#include "boost/thread.hpp"
#include "boost/thread/future.hpp"

#include <algorithm>
#include <future>
#include <memory>
#include <string>
#include <utility>

// #include "city.h"
// #include "libcuckoo/cuckoohash_map.hh"
// #include "libcuckoo/city_hasher.hh"

namespace utils {
    template <typename Container>
    Container read_baseline(const std::string &database,
                            const std::vector<std::string> &folders, bool verbose = false) {
        using IArchive = utils::DefaultIArchive;
        Container allFiles;

        // utils::ElapsedTime<utils::MILLISECOND> t("Read baseline: ");

        // Open the database
        std::unique_ptr<rocksdb::DB> db(utils::open(database));

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

            /**
             * Read vertex and graph information using two threads.
             *
             */

            // Read vertex ids
            auto readVidObj = [&db]() -> std::vector<std::string> {
                std::vector<std::string> vids;
                std::string value;
                rocksdb::Status s = db->Get(rocksdb::ReadOptions(), Resources::VIDKey, &value);
                assert(s.ok());
                std::istringstream is(value);
                IArchive input(is);
                input(vids);
                return vids;
            };

            // Read graph info
            auto readGraphObj = [&db]() -> Graph {
                Graph g;
                std::string value;
                rocksdb::Status s =
                    db->Get(rocksdb::ReadOptions(), utils::Resources::GraphKey, &value);
                assert(s.ok());
                std::istringstream is(value);
                IArchive input(is);
                input(g);
                return g;
            };

            using namespace boost;
            future<std::vector<std::string>> t1 = async(readVidObj);
            future<Graph> t2 = async(readGraphObj);

            t1.wait();
            t2.wait();

            std::vector<std::string> vids = t1.get();
            Graph g = t2.get();

            if (verbose) {
                fmt::print("Number of vertexes: {0}\n", g.numberOfVertexes());
            }

            assert(vids.size() == g.numberOfVertexes());

            /**
             * Find all vertexes that belong to given folders.
             *
             */

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

            std::vector<index_type> allVids =
                graph::dfs_preordering<std::vector<index_type>>(g, indexes);

            // Need to sort vids to maximize the read performance.
            tbb::parallel_sort(allVids.begin(), allVids.end());

            {
                std::sort(allVids.begin(), allVids.end());
                const auto readOpts = rocksdb::ReadOptions();
                for (auto const &index : allVids) {
                    const std::string aKey = utils::to_fixed_string(9, index);
                    std::string value;
                    auto const s = db->Get(readOpts, aKey, &value);
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

            // Now read all keys and create a list of edited file data
            // bases. The comlexity of this algorithm is O(n) because allVids
            // and keys are sorted. However, this algorithm might be slower than
            // the above algoritm in practice. Need to find a better approach.
            // if (!allVids.empty()) {
            //       size_t idx = 0;
            //       std::unique_ptr<rocksdb::Iterator>
            //       it(db->NewIterator(rocksdb::ReadOptions()));
            //       for (it->SeekToFirst(); it->Valid(); it->Next()) {
            //           const std::string aKey = it->key().ToString();
            //           std::string expectedKey = utils::to_fixed_string(9, allVids[idx]);
            //           if (expectedKey == aKey) {
            //               const std::string value = it->value().ToString();
            //               std::istringstream is(value);

            //               Vertex<index_type> aVertex;
            //               {
            //                   IArchive input(is);
            //                   input(aVertex);
            //               }
            //               std::move(aVertex.Files.begin(), aVertex.Files.end(),
            //                         std::back_inserter(allFiles));
            //               ++idx;

            //               if (idx == allVids.size()) {
            //                   break;
            //               }
            //           }
            //       }
            //   }
        }

        // fmt::print("Number of files: {}\n", allFiles.size());

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
    std::tuple<Container, Container, Container> diff(Container &&first, Container &&second,
                                                     bool verbose = false) {
        utils::ElapsedTime<utils::MILLISECOND> t("Diff time: ");
        Container modifiedFiles;
        Container newFiles;
        Container deletedFiles;

        // Create a lookup table
        using value_type = typename Container::value_type;

        using Dict = std::unordered_set<FileInfo>;
        Dict dict(second.begin(), second.end());
        Container results;

        if (verbose) {
            std::cout << "---- Dictionary sizes: " << dict.size() << " \n";
        }

        // Remove items that belong to both. We assume that the number of
        // modified files is significantly less than the total number of files.
        auto getDiff = [&dict, &results, &first](const int idx) {
            auto const & aKey = first[idx];
            if (dict.find(aKey) == dict.end()) {
                dict.erase(aKey);
            } else {
                results.push_back(aKey);
            }
        };

        int N = first.size();
        tbb::parallel_for(0, N, 1, getDiff);

        // Get modified and deleted items.
        std::unordered_map<std::string, value_type> map;
        map.reserve(dict.size());
        for (auto it = dict.begin(); it != dict.end(); ++it) {
        }
        // for (size_t idx = 0; idx < dict.size(); ++ idx) {
        //     auto const item = dict.keys(idx);
        //     map.emplace(std::make_pair(item.Path, item));
        // }

        // // Note: Modified files are files that are also in the dictionary,
        // // however, they have a different size and permission.
        // if (!map.empty()) {
        //     for (auto item : results) {
        //         const auto pos = map.find(item.Path);
        //         if (pos != map.end()) {
        //             auto dictItem = pos->second;
        //             bool isOK = (item.Size == dictItem.Size) &&
        //                         (item.Permissions == dictItem.Permissions);
        //             if (!isOK) {
        //                 modifiedFiles.emplace_back(item);
        //             }
        //             map.erase(item.Path);
        //         } else {
        //             newFiles.emplace_back(item);
        //         }
        //     }
        // } else {
        //     newFiles = std::move(results);
        // }

        // // Get deleted items
        // std::for_each(map.begin(), map.end(), [&deletedFiles](auto const &item) {
        //     deletedFiles.emplace_back(item.second);
        // });

        // Return
        return std::make_tuple(modifiedFiles, newFiles, deletedFiles);
    }
    
    auto diffFolders(const std::string &dataFile, const std::vector<std::string> &folders,
                     bool verbose) {
        // Search for files in the given folders.
        using path = boost::filesystem::path;
        using PathContainer = std::vector<path>;
        using Container = std::vector<utils::FileInfo>;

        PathContainer searchFolders;
        for (auto item : folders) {
            searchFolders.emplace_back(path(item));
        }

        // We do real work here
        using Visitor =
            utils::filesystem::SimpleVisitor<PathContainer, utils::filesystem::NormalPolicy>;
        auto searchObj = [&searchFolders]() -> Container {
            Visitor visitor;
            utils::filesystem::dfs_file_search(searchFolders, visitor);
            return visitor.getResults();
        };
        auto readObj = [dataFile, &folders, verbose]() {
            return utils::read_baseline<Container>(dataFile, folders, verbose);
        };

        using namespace boost;
        future<Container> readThread = async(readObj);
        future<Container> findThread = async(searchObj);

        readThread.wait();
        findThread.wait();
        Container baseline = readThread.get();
        Container results = findThread.get();

        // Return the differences between baseline and current state.
        if (verbose) {
            fmt::print("Number of files: {}\n", results.size());
            fmt::print("Number of files in the baseline: {}\n", baseline.size());
        }

        return utils::diff(std::move(baseline), std::move(results));
    }
}
#endif
