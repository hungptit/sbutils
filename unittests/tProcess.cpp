// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <fstream>
#include <iostream>

#include "sbutils/TemporaryDirectory.hpp"
#include "sbutils/Process.hpp"

#define CATCH_CONFIG_MAIN
#include "catch/catch.hpp"

TEST_CASE("Basic", "Positive") {
    sbutils::TemporaryDirectory tmpDir;
    std::string cmd("/bin/ls");
    std::vector<std::string> args = {"-l"};
    args.emplace_back(tmpDir.getPath().string());
    std::string buffer;
    sbutils::run(cmd, args, buffer);
    std::cout << "buffer: \"" << buffer << "\"";
    CHECK(buffer == sbutils::run(cmd, args));
    CHECK(buffer == "total 0\n");
}
