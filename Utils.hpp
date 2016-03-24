#ifndef Utils_hpp_
#define Utils_hpp_

#include <string>
#include <tuple>

namespace utils {
    // Define the file information which is (path, stem, extension, permission,
    // time_stamp, file_size)
    using FileInfo = std::tuple<std::string, std::string, std::string, int,
                                std::time_t, uintmax_t>;
}

#endif
