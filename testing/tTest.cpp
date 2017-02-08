#include "gtest/gtest.h"
#include <array>
#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "boost/config.hpp"
#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"

#include "FileSearch.hpp"
#include "Timer.hpp"
#include "LookupTable.hpp"
#include "Print.hpp"

#include <sstream>
#include <string>
#include <vector>

#include "gtest/gtest.h"

template <typename Container>
auto createTestData() {
    Container data = {std::make_tuple("foo", 1), std::make_tuple("boo", 2),
                      std::make_tuple("foo", 2), std::make_tuple("foo", 3)};
    return data;
}

int main() {
    {
        using Container = std::vector<std::tuple<std::string, size_t>>;
        auto data = createTestData<Container>();

        {
            utils::LookupTable<Container> table(data);
            table.disp();
            // auto results = table.getIdx("foo");
            // std::cout << "--- Results ---\n";
            // std::for_each(std::get<0>(results), std::get<1>(results),
            //               [](auto &val) { std::cout << val << "\n"; });
            
            // std::cout << table.get(3) << "\n";
            // std::cout << table.get(2) << "\n";
            // std::cout << table.get(0) << "\n";
        }

        {
            utils::LookupTable<Container, 1> table(data);
            table.disp();

            std::cout << "--- Results ---\n";
            auto results = table.getIdx(2);
            std::for_each(std::get<0>(results), std::get<1>(results),
                          [](auto &val) { std::cout << val << "\n"; });

            // std::cout << table.get(3) << "\n";
            // std::cout << table.get(2) << "\n";
            // std::cout << table.get(0) << "\n";
        }
    }
}
