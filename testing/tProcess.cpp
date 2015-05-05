#include <fstream>
#include <iostream>
#include "utils/Utils.hpp"
#include "utils/Process.hpp"
#include "gtest/gtest.h"
TEST(Basic, Positive) {
    Tools::TemporaryDirectory tmpDir;
    std::string cmd("/bin/ls");
    std::vector<std::string> args = {"-l"};
    args.emplace_back(tmpDir.getPath().string());
    std::string buffer;
    Tools::run(cmd, args, buffer);
    std::cout << "buffer: \"" << buffer << "\"";
    EXPECT_TRUE(buffer == Tools::run(cmd, args));
    EXPECT_TRUE(buffer == "total 0\n");
}
