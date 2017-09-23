#include "fmt/format.h"
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "fb.hpp"

#include "celero/Celero.h"

constexpr int NumberOfSamples = 200000;
constexpr int NumberOfIterations = 10;

std::string aLine("fdsf "
                  "ggssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
                  "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
                  "sssssssssssssssssssssssssssssssfdgfd  gfdg f gfdgdf gd ");
folly::fbstring aLinefb(aLine);

const char pattern[] = {"Hello fdsfdsf  fds fds  fdsf dsd"};

std::string stdpattern(pattern);
folly::fbstring fbpattern(pattern);

template <typename String> bool search(const String &str, const String &pattern) {
    return str.find(pattern) != String::npos;
}

CELERO_MAIN

BASELINE(search, std_string, NumberOfSamples, NumberOfIterations) {
    celero::DoNotOptimizeAway(search<std::string>(aLine, stdpattern));
}

BENCHMARK(search, fbstring1, NumberOfSamples, NumberOfIterations) {
    celero::DoNotOptimizeAway(search<folly::fbstring>(aLinefb, pattern));
}

BENCHMARK(search, fbstring2, NumberOfSamples, NumberOfIterations) {
    celero::DoNotOptimizeAway(search<folly::fbstring>(aLinefb, fbpattern));
}

// int main() {
// 	folly::fbstring str("Hello");
// 	fmt::print("{}\n", str.data());
// 	std::unordered_set<folly::fbstring> dict;

// 	std::ofstream os("out.cereal", std::ios::binary);
// 	cereal::BinaryOutputArchive archive( os );
// 	archive(str);
// }
