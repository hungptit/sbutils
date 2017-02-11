#include <fstream>
#include <iostream>
<<<<<<< HEAD
#include "sbutils/TemporaryDirectory.hpp"
#include "sbutils/Process.hpp"
=======
#include "TemporaryDirectory.hpp"
#include "Process.hpp"
>>>>>>> 38bd3e84850c86b3842f2ac8a3e044da15af8688
#include "gtest/gtest.h"

TEST(Basic, Positive) {
    utils::TemporaryDirectory tmpDir;
    std::string cmd("/bin/ls");
    std::vector<std::string> args = {"-l"};
    args.emplace_back(tmpDir.getPath().string());
    std::string buffer;
    utils::run(cmd, args, buffer);
    std::cout << "buffer: \"" << buffer << "\"";
    EXPECT_TRUE(buffer == utils::run(cmd, args));
    EXPECT_TRUE(buffer == "total 0\n");
}
