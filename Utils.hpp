#ifndef Utils_hpp_
#define Utils_hpp_

#include <string>
#include <tuple>
#include <iomanip>
#include <iostream>

namespace utils {
    // Define the file information which is (path, stem, extension, permission,
    // time_stamp, file_size)
    using FileInfo = std::tuple<std::string, std::string, std::string, int,
                                std::time_t, uintmax_t>;

    template <typename T> std::string to_fixed_string(const int len, T val) {
        std::ostringstream ss;
        ss << std::setw(len) << std::setfill('0') << val;
        return ss.str();
    }
}

#endif
