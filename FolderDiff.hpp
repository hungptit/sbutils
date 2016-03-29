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

#include "SparseGraph.hpp"

namespace utils {
    /**
     * This function assumes that given keys are sorted.
     *
     * @param reader
     * @param keys
     *
     * @return
     */
    template <typename Container, typename IArchive>
    Container load_baseline(Reader &reader,
                            const std::vector<std::string> &keys) {
        auto Database = reader.getDB();
        Container results;
        leveldb::Iterator *it = Database->NewIterator(leveldb::ReadOptions());

        auto currentKey = keys.begin();
        for (it->SeekToFirst(); it->Valid(); it->Next()) {
            if (currentKey == keys.end()) {
                break;
            }

            if (it->key().ToString() == *currentKey) {
                // Deserialize the data
                Container data;
                std::istringstream is(it->value().ToString());
                IArchive input(is);
                input(data);
                std::move(data.begin(), data.end(),
                          std::back_inserter(results));

                // Advance to the next key
                ++currentKey;
            }

            if (!it->status().ok()) {
                std::cerr << "An error was found during the scan" << std::endl;
                std::cerr << it->status().ToString() << std::endl;
            }
        }

        // Cleanup it;
        delete it;

        // Return
        return results;
    }

    template <typename Container, typename ExtContainer, typename StemContainer>
    auto filter(Container &data, ExtContainer &exts, StemContainer &stems) {
        // utils::ElapsedTime<utils::MILLISECOND> t1("Filter time: ");
        utils::filesystem::ExtFilter<ExtContainer> f1(exts);
        utils::filesystem::StemFilter<StemContainer> f2(stems);
        return utils::filesystem::filter(data.begin(), data.end(), f1, f2);
    }

    template <typename Container>
    Container read_baseline(utils::Reader &reader,
                            const std::vector<std::string> &folders,
                            bool verbose = false) {
        using IArchive = utils::DefaultIArchive;
        Container allFiles;

        if (folders.empty()) {
            // utils::ElapsedTime<utils::MILLISECOND> t("Deserialization
            // time:
            // ");
            std::istringstream is(reader.read(utils::Resources::AllFileKey));
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
            using edge_type = int;
            using GraphAlg = utils::SparseGraph<index_type, edge_type>;
            using vertex_container = typename GraphAlg::VertexContainer;
            using edge_container = typename GraphAlg::EdgeContainer;
            using index_type = typename GraphAlg::index_type;

            // Only read the file information for given folders.
            std::istringstream is(reader.read(utils::Resources::GraphKey));
            std::vector<std::string> vids;
            vertex_container v;
            edge_container e;
            IArchive input(is);
            input(vids, v, e);
            GraphAlg g(v, e, true);

            if (verbose) {
                fmt::print("Number of vertexes: {0}\n", vids.size());
                fmt::print("Number of edges: {0}\n", e.size());
            }

            // Display detail information about file hierarchy tree.
            // tree_info(g, vids);

            // Now find indexes for given folders
            std::vector<index_type> indexes;
            for (auto item : folders) {
                auto aKey = utils::normalize_path(item);
                // fmt::print("sKey = {}\n", aKey);
                auto it = std::lower_bound(vids.begin(), vids.end(), aKey);
                if (*it == aKey) {
                    auto vid = std::distance(vids.begin(), it);
                    indexes.push_back(vid);
                } else {
                    fmt::print("Could not find key {} in database\n", item);
                }
            }

            // Use DFS to find all vertexes that are belong to given
            // vertexes
            utils::graph::NormalVisitor<GraphAlg, std::vector<index_type>>
                visitor(vids.size());
            utils::graph::dfs(g, visitor, indexes);
            auto results = visitor.getResults();

            // Now read all keys and create a list of edited file data
            // bases.
            {
                std::sort(results.begin(), results.end());
                std::vector<std::string> keys;
                for (auto index : results) {
                    keys.emplace_back(utils::to_fixed_string(9, index));
                }

                allFiles =
                    utils::load_baseline<Container, IArchive>(reader, keys);
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
    std::tuple<Container, Container, Container> diff(const Container &first,
                                                     const Container &second,
                                                     bool verbose = false) {

        Container modifiedFiles;
        Container results;
        Container newFiles;
        Container deletedFiles;

        // Create a lookup table
        using value_type = typename Container::value_type;

        boost::unordered_set<value_type> dict(second.begin(), second.end());

        if (verbose) {
            std::cout << "---- Dictionary sizes: " << dict.size() << " \n";
        }

        // Cleanup items that belong to both.
        // The complexity of this algorithm is O(n).
        for (auto item : first) {
            auto pos = dict.find(item);
            if (pos == dict.end()) {
                results.emplace_back(item);
            } else {
                dict.erase(pos);
            }
        }

        // Get modified and deleted items.
        boost::unordered_map<std::string, value_type> map;
        for (auto item : dict) {
            map[std::get<0>(item)] = item;
        }

        // Note: Modified files are files that are also in the dictionary,
        // however, they have a different size and permission.
        if (!map.empty()) {
            for (auto item : results) {
                auto aKey = std::get<0>(item);
                auto pos = map.find(aKey);
                if (pos != map.end()) {
                    auto dictItem = pos->second;
                    bool isOK =
                        (std::get<utils::filesystem::FILESIZE>(item) ==
                         std::get<utils::filesystem::FILESIZE>(dictItem)) &&
                        (std::get<utils::filesystem::PERMISSION>(item) ==
                         std::get<utils::filesystem::PERMISSION>(dictItem));
                    if (!isOK) {
                        modifiedFiles.emplace_back(item);
                    }
                    map.erase(aKey);
                } else {
                    newFiles.emplace_back(item);
                }
            }
        } else {
            newFiles = std::move(results);
        }

        // Get deleted items
        for (auto item : map) {
            deletedFiles.emplace_back(item.second);
        }

        // Return
        return std::make_tuple(modifiedFiles, newFiles, deletedFiles);
    }

    auto diffFolders(const std::string &dataFile,
                     std::vector<std::string> &folders, bool verbose) {
        // Search for files in the given folders.
        using path = boost::filesystem::path;
        using PathContainer = std::vector<path>;
        using Container = std::vector<utils::FileInfo>;

        utils::filesystem::SimpleVisitor<PathContainer,
                                         utils::filesystem::NormalPolicy>
            visitor;
        PathContainer searchFolders;
        for (auto item : folders) {
            searchFolders.emplace_back(path(item));
        }

        utils::Reader reader(dataFile);

        // We do real work here
        auto searchObj = [&searchFolders, &visitor]() {
            utils::filesystem::dfs_file_search(searchFolders, visitor);
        };
        auto readObj = [&reader, &folders, verbose]() {
            return utils::read_baseline<Container>(reader, folders, verbose);
        };

        // utils::filesystem::dfs_file_search(searchFolders, visitor);
        // auto baseline = utils::read_baseline<Container>(reader, folders,
        // false);

        // auto baseline = readObj();
        // searchObj();

        boost::future<Container> readThread = boost::async(readObj);
        boost::future<void> findThread = boost::async(searchObj);;

        readThread.wait();
        findThread.wait();
        auto baseline = readThread.get();
        findThread.get();

        // Return the differences between baseline and current state.
        auto const &results = visitor.getResults();
        if (verbose) {
            fmt::print("Number of files: {}\n", results.size());
            fmt::print("Number of files in the baseline: {}\n", results.size());
        }

        return utils::diff(baseline, results);
    }
}
#endif
