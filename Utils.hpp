#ifndef Utils_hpp_
#define Utils_hpp_

#include "FileUtils.hpp"
#include "Serialization.hpp"
#include "Print.hpp"

namespace utils {
  // Define the file information which is (path, stem, extension, permission,
  // time_stamp)
  using FileInfo = std::tuple<std::string, std::string, std::string, int, std::time_t>;
}

#endif
