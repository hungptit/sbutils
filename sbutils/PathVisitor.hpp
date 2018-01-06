#pragma once

// STL headers
#include <string>
#include <vector>

// Other headers
#include "boost/filesystem.hpp"
#include "fmt/format.h"
#include "graph/graph.hpp"

namespace sbutils {
    // TODO: Tune the performance of this struct.
    struct FileProperties {
        int Permissions;
        boost::filesystem::file_type Type;
        size_t Size;
        std::time_t TimeStamp;

        template <typename Archive> void serialize(Archive &ar) {
            ar(Permissions, Type, Size, TimeStamp);
        }
    };
}
