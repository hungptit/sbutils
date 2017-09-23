#include "fmt/format.h"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <unordered_set>

#include "fb.hpp"

int main() {
	folly::fbstring str("Hello");
	fmt::print("{}\n", str.data());
	std::unordered_set<folly::fbstring> dict;

	std::ofstream os("out.cereal", std::ios::binary);
	cereal::BinaryOutputArchive archive( os );
	archive(str);

	fmt::print("jemalloc: {}\n", folly::usingJEMalloc() ? "true" : "false");
}
