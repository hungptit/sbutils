#ifndef Json_hpp
#define Json_hpp

#include <string>
#include "boost/lexical_cast.hpp"

namespace Json {
std::string toJSON(const std::string &fieldName, const std::string &data) {
        { return "\"" + fieldName + "\":\"" + data + "\""; }
}

}
#endif
