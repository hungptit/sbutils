#ifndef DatabaseUtils_hpp_
#define DatabaseUtils_hpp_

#include "FileSearch.hpp"
#include "LevelDBIO.hpp"
#include "utils/Resources.hpp"
#include "utils/Serialization.hpp"

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
        Container load(Reader &reader,
                       const std::vector<std::string> &keys) {
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
    }
}

#endif
