#ifndef LookupTable_hpp
#define LookupTable_hpp

#include <algorithm>
#include <unordered_map>

namespace utils {
    template <typename Container, int main_key_idx = 0> class LookupTable {
      public:
        using value_type = typename Container::value_type;
        using key_type =
            typename std::tuple_element<main_key_idx, value_type>::type;
        using index_type = size_t;

        LookupTable(Container data) : Table(data) {
            HashTable.reserve(Table.size());
            index_type id = 0;
            for (auto &item : Table) {
                HashTable.emplace(
                    std::make_pair(std::get<main_key_idx>(item), id++));
            }
        }

        /**
         * Return a range of found values.
         *
         * @param aKey
         *
         * @return
         * @note Complexity: Constant on average, worst case linear in the size
         * of the container.
         */
        auto getIdx(const key_type &aKey) {
            return std::make_tuple(HashTable.find(aKey), HashTable.end());
        }

        /**
         * Return value for a given ID.
         *
         * @param idx
         *
         * @return
         */
        const value_type get(const index_type idx) {
            assert(idx < Table.size());
            return Table[idx];
        }

        Container & data() const {
            return Table;
        }


        void disp() {
            std::cout << "--- Table ---\n";
            std::for_each(Table.begin(), Table.end(),
                          [](auto &val) { std::cout << val << "\n"; });
            std::for_each(HashTable.begin(), HashTable.end(),
                          [](auto &val) { std::cout << val << "\n"; });
        }

      private:
        Container Table;
        std::unordered_multimap<key_type, index_type> HashTable;
    };
}
#endif
