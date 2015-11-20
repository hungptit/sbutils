#ifndef Utils_hpp_
#define Utils_hpp_

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

#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/archives/xml.hpp>

#include "BasicUtilities.hpp"
#include "FileSystemUtilities.hpp"
#include "TimeUtilities.hpp"

namespace Utils {
    // Define the default serilized formats
    typedef cereal::BinaryOutputArchive OArchive;
    typedef cereal::BinaryInputArchive IArchive;

    // Define the file information which is (path, stem, extension, permission,
    // time_stamp)
    typedef std::tuple<std::string, std::string, std::string, int, std::time_t>
        FileInfo;
}

#endif
