#include <algorithm>
#include <chrono>
#include <future>
#include <iostream>
#include <random>
#include <type_traits>
#include <vector>
#include <iostream>
#include "blaze/Blaze.h"

#include "celero/Celero.h"

constexpr int NumberOfSamples = 20;
constexpr int NumberOfIterations = 10;

template <typename Container>
double sum(const Container &v, const std::size_t begin, const std::size_t end) {
    double s = 0;
    for (auto idx = begin; idx < end; ++idx) {
        s += v[idx] * v[idx];
    }
    return s;
}

template <typename Container>
double sum_async(const Container &v, const std::size_t begin, const std::size_t end) {
    auto middle = (begin + end) / 2;
	// Removing std::cref will slow down this function because it makes two copy of v..
    auto s1 = std::async(std::launch::async, sum<Container>, std::cref(v), begin, middle);
    auto s2 = std::async(std::launch::async, sum<Container>, std::cref(v), middle, end);
    return s1.get() + s2.get();
}

template <typename Iter>
typename std::iterator_traits<Iter>::value_type sum_iter(Iter begin, Iter end) {
    typename std::iterator_traits<Iter>::value_type results = 0.0;
    std::for_each(begin, end, [&results](auto const item) { results += item * item; });
    return results;
}

template <typename Iter>
typename std::iterator_traits<Iter>::value_type sum_iter_async(Iter begin, Iter end) {
    Iter middle = begin + std::distance(begin, end) / 2;
    auto s1 = std::async(std::launch::async, sum_iter<Iter>, begin, middle);
    auto s2 = std::async(std::launch::async, sum_iter<Iter>, middle, end);
    return s1.get() + s2.get();
}

template <typename T> auto create_test_data(const size_t N) {
    auto s = std::chrono::system_clock::now().time_since_epoch().count();
    std::vector<T> v(N);
    std::generate(v.begin(), v.end(), std::minstd_rand0(s));
    return v;
}

template <typename T> auto create_blaze_test_data(const std::vector<T> &data) {
  using blaze::DynamicVector;
  DynamicVector<T> results(data.size());
  for (size_t idx =  0; idx < data.size(); ++idx) {
    results[idx] = data[idx];
  }
  return results;
}

template <typename T> auto sum_blaze(const T & x, const size_t nthreads = 1) {
  blaze::setNumThreads( nthreads );
  return blaze::dot(x, x);
}

// Create test data
constexpr size_t N = 100000;
using value_type = double;
auto data = create_test_data<value_type>(N);
auto blaze_data = create_blaze_test_data(data);
using container_type = decltype(data);

using blaze::StaticVector;
using blaze::DynamicVector;

CELERO_MAIN

BASELINE(sum, orginal, NumberOfSamples, NumberOfIterations) {
    celero::DoNotOptimizeAway(sum<container_type>(data, 0, N));
}

BENCHMARK(sum, orginal_async, NumberOfSamples, NumberOfIterations) {
    celero::DoNotOptimizeAway(sum_async<container_type>(data, 0, N));
}

BENCHMARK(sum, iter, NumberOfSamples, NumberOfIterations) {
    celero::DoNotOptimizeAway(sum_iter(data.cbegin(), data.cend()));
}

BENCHMARK(sum, iter_async, NumberOfSamples, NumberOfIterations) {
    celero::DoNotOptimizeAway(sum_iter_async(data.cbegin(), data.cend()));
}

BENCHMARK(sum, blaze, NumberOfSamples, NumberOfIterations) {
  celero::DoNotOptimizeAway(sum_blaze(blaze_data, 1));
}

BENCHMARK(sum, blaze_two_threads, NumberOfSamples, NumberOfIterations) {
  celero::DoNotOptimizeAway(sum_blaze(blaze_data, 2));
}

BENCHMARK(sum, blaze_four_threads, NumberOfSamples, NumberOfIterations) {
  celero::DoNotOptimizeAway(sum_blaze(blaze_data, 4));
}
