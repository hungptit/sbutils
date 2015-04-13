#ifndef Basic_hpp_
#define Basic_hpp_

#include <iostream>
#include <string>
#include <cstdlib>

namespace Tools {
template <typename Data> void disp(Data &data, const std::string &message) {
    std::cout << message << "[ ";
    for (const auto &val : data) {
        std::cout << val << " ";
    }
    std::cout << "]" << std::endl;
}

template <typename Data>
void disp_pair(Data &data, const std::string &message) {
    std::cout << message << "[ ";
    for (const auto &val : data) {
        std::cout << "(" << val.first << "," << val.second << ") ";
    }
    std::cout << "]" << std::endl;
}

/**
 * @todo Improve this function!
 */
int run(const std::string &command) { return std::system(command.c_str()); }
}

#endif
