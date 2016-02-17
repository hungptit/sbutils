#ifndef FolderDiff_hpp
#define FolderDiff_hpp

#include "boost/unordered_map.hpp"
#include "boost/unordered_set.hpp"

namespace utils {
    template <typename Container> class FolderDiff {
      public:
        template <typename FileSearch>
        void find(FileSearch &fsearch, const std::string &aFolder) {
            fsearch.search(aFolder);
            // std::cout << "Found files: " << fsearch.getData().size() << "\n";
        }

        template <typename Reader>
        Container read(Reader &reader, const std::string &aKey) {
            Container dict;
            auto const results = reader.read(aKey);
            if (!results.empty()) {
                std::istringstream is(results);
                utils::load<utils::IArchive, Container>(dict, is);
            }
            // std::cout << "Dictionary sizes: " << dict.size() << "\n";
            return dict;
        }

        /**
         * This methods will return a tuple which has
         *     1. Items in first and not in second.
         *     2. Items in both first and second but they are different.
         *     3. Items which are in second and not in first.
         * The first item of each data element is a using string which is a full
         * file name.
         */
        std::tuple<Container, Container, Container>
        diff(const Container &first, const Container &second,
             bool verbose = false) {

            Container modifiedFiles;
            Container results;
            Container newFiles;
            Container deletedFiles;

            // Create a lookup table
            typedef typename Container::value_type value_type;
            boost::unordered_set<value_type> dict(second.begin(), second.end());

            if (verbose) {
                std::cout << "---- Dictionary sizes: " << dict.size() << " \n";
            }

            // Cleanup iteam which belong to both.
            // The complexity of this algorithm is O(cn).
            for (auto item : first) {
                auto pos = dict.find(item);
                if (pos == dict.end()) {
                    // modified files and new files will be stored in results.
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

            if (!map.empty()) {
                for (auto item : results) {
                    auto aKey = std::get<0>(item);
                    auto pos = map.find(aKey);
                    if (pos != map.end()) {
                        modifiedFiles.emplace_back(item);
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
    };
}
#endif
