#ifndef DatabaseUtils_hpp_
#define DatabaseUtils_hpp_

#include "FileSearch.hpp"
#include "FileUtils.hpp"
#include "LevelDBIO.hpp"
#include "Resources.hpp"
#include "Serialization.hpp"

namespace utils {
    namespace Database {

        /**
         * This function assumes that given keys are sorted.
         *
         * @param reader
         * @param keys
         *
         * @return
         */
        template <typename Container, typename IArchive>
        Container load(Reader &reader, const std::vector<std::string> &keys) {
            auto Database = reader.getDB();
            Container results;
            leveldb::Iterator *it =
                Database->NewIterator(leveldb::ReadOptions());

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
                    std::cerr << "An error was found during the scan"
                              << std::endl;
                    std::cerr << it->status().ToString() << std::endl;
                }
            }

            // Cleanup it;
            delete it;

            // Return
            return results;
        }

        template <typename Graph>
        void tree_info(const Graph &g, std::vector<std::string> &vids) {
            utils::graph::graph_info(g);
            fmt::MemoryWriter writer;
            size_t counter = 0;
            for (auto item : vids) {
                writer << "vid[" << counter << "] = " << item << "\n";
                counter++;
            }
            std::cout << writer.str();
        }

        template <typename Container, typename ExtContainer,
                  typename StemContainer>
        auto filter(Container &data, ExtContainer &exts, StemContainer &stems) {
            // utils::ElapsedTime<utils::MILLISECOND> t1("Filter time: ");
            utils::filesystem::ExtFilter<ExtContainer> f1(exts);
            utils::filesystem::StemFilter<StemContainer> f2(stems);
            return utils::filesystem::filter(data.begin(), data.end(), f1, f2);
        }

        template <typename Container>
        Container
        read(utils::Reader &reader, const std::vector<std::string> &folders,
             bool verbose = false) {
            using IArchive = utils::DefaultIArchive;
            Container allFiles;

            if (folders.empty()) {
                // utils::ElapsedTime<utils::MILLISECOND> t("Deserialization
                // time:
                // ");
                std::istringstream is(
                    reader.read(utils::Resources::AllFileKey));
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

                // utils::DFS<GraphAlg> alg;
                // auto results = alg.dfs(g, indexes);

                // Now read all keys and create a list of edited file data
                // bases.
                {
                    std::sort(results.begin(), results.end());
                    std::vector<std::string> keys;
                    for (auto index : results) {
                        keys.push_back(utils::to_fixed_string(9, index));
                    }

                    allFiles = utils::Database::load<Container, IArchive>(
                        reader, keys);
                }
            }

            return allFiles;
        }
    }
}

#endif
