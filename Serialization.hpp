#ifndef Serialization_hpp_
#define Serialization_hpp_

#include <deque>
#include <list>
#include <vector>

#include <map>
#include <set>

#include <unordered_map>
#include <unordered_set>

#include <array>
#include <string>

// Cereal
#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/archives/xml.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/bitset.hpp>
#include <cereal/types/chrono.hpp>
#include <cereal/types/complex.hpp>
#include <cereal/types/deque.hpp>
#include <cereal/types/forward_list.hpp>
#include <cereal/types/list.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/types/queue.hpp>
#include <cereal/types/set.hpp>
#include <cereal/types/stack.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/tuple.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/unordered_set.hpp>
#include <cereal/types/utility.hpp>
#include <cereal/types/valarray.hpp>
#include <cereal/types/vector.hpp>

namespace utils {
    // Define the default serilized formats
    typedef cereal::BinaryOutputArchive OArchive;
    typedef cereal::BinaryInputArchive IArchive;

    template <typename OArchive, typename Container>
    void save(const Container &data, std::ostringstream &os) {
        OArchive oar(os);
        oar(cereal::make_nvp("data", data));
    }

    template <typename OArchive, typename Container>
    void load(Container &data, std::istringstream &is) {
        OArchive iar(is);
        iar(data);
    }
}

#endif
