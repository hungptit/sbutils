// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
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

#define BOOST_THREAD_VERSION 4
#include "boost/config.hpp"
#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"
#include "boost/thread.hpp"
#include "boost/thread.hpp"
#include "boost/thread/future.hpp"

#include "utils/FileSearch.hpp"
#include "utils/FolderDiff.hpp"
#include "utils/LookupTable.hpp"
#include "utils/Timer.hpp"
#include "utils/Utils.hpp"

#include <sstream>
#include <string>
#include <vector>

#include "gtest/gtest.h"



template <typename Container> auto createTestData() {
    Container data = {std::make_tuple("foo", 1), std::make_tuple("boo", 2),
                      std::make_tuple("foo", 2), std::make_tuple("foo", 3)};
    return data;
}

TEST(BasicTests, Positive) {
    using Container = std::vector<std::tuple<std::string, size_t>>;
    auto data = createTestData<Container>();

    {
        utils::LookupTable<Container> table(data);
        table.disp();
        auto results = table.getIdx("foo");
        auto begin = std::get<0>(results), end = std::get<1>(results);
        std::cout << "--- Results ---\n";
        std::for_each(begin, end, [](auto &val) { std::cout << val << "\n"; });

        std::cout << table.get(3) << "\n";
        std::cout << table.get(2) << "\n";
        std::cout << table.get(0) << "\n";
    }
}
