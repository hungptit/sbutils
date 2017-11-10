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

// constexpr int NumberOfSamples = 20;
// constexpr int NumberOfIterations = 10;

template <typename Container>
double sum(const Container &v, const std::size_t begin, const std::size_t end) {
    double s = 0;
    for (auto idx = begin; idx < end; ++idx) {
        s += v[idx];
    }
    return s;
}

template <typename Container>
double sum_async(const Container &v, const std::size_t l, const std::size_t h) {
    auto m = (l + h) / 2;
    auto s1 = std::async(std::launch::async, sum<Container>, std::cref(v), l, m);
    auto s2 = std::async(std::launch::async, sum<Container>, std::cref(v), m + 1, h);
    return s1.get() + s2.get();
}

template <typename Iter> typename Iter::value_type sum_iter(Iter begin, Iter end) {
    typename Iter::value_type result;
    std::for_each(begin, end, [&result](auto const item) { sum += item; });
    return result;
}

template<typename T>
auto create_test_data(const size_t N) {
    auto s = std::chrono::system_clock::now().time_since_epoch().count();
    std::vector<T> v(N);
    std::generate(v.begin(), v.end(), std::minstd_rand0(s));
    return v;
}

constexpr size_t N = 1000000;
auto data = create_test_data<double>(N);
using container_type = decltype(data);


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


// int main() {
// 	folly::fbstring str("Hello");
// 	fmt::print("{}\n", str.data());
// 	std::unordered_set<folly::fbstring> dict;

// 	std::ofstream os("out.cereal", std::ios::binary);
// 	cereal::BinaryOutputArchive archive( os );
// 	archive(str);
// }
