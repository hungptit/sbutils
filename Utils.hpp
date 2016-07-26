#ifndef Utils_hpp_
#define Utils_hpp_

#include <string>
#include <tuple>
#include <iomanip>
#include <iostream>

#include "DataStructures.hpp"

namespace utils {
    // Define the file information which is (path, stem, extension, permission,
    // time_stamp, file_size)
       
    template <typename T> std::string to_fixed_string(const int len, T val) {
        std::ostringstream ss;
        ss << std::setw(len) << std::setfill('0') << val;
        return ss.str();
    }
}

#endif
