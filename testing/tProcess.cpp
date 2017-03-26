#include <fstream>
#include <iostream>

#include "sbutils/TemporaryDirectory.hpp"
#include "sbutils/Process.hpp"
#include "gtest/gtest.h"

TEST(Basic, Positive) {
    sbutils::TemporaryDirectory tmpDir;
    std::string cmd("/bin/ls");
    std::vector<std::string> args = {"-l"};
    args.emplace_back(tmpDir.getPath().string());
    std::string buffer;
    sbutils::run(cmd, args, buffer);
    std::cout << "buffer: \"" << buffer << "\"";
    EXPECT_TRUE(buffer == sbutils::run(cmd, args));
    EXPECT_TRUE(buffer == "total 0\n");
}
