#include "fmt/format.h"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <future>
#include <random>

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

BASELINE(sum, original, 5, 10) {
    celero::DoNotOptimizeAway(sum(data, 0, N));
}
