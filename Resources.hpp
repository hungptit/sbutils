#ifndef Resources_hpp_
#define Resources_hpp_

#include <string>

namespace utils {
    struct FileDatabaseInfo {
        static const std::string Database;
    };
    const std::string FileDatabaseInfo::Database = ".database";
}

#endif
