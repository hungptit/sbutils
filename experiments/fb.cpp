#include <iostream>
#include <stdexcept>
#include <fstream>
#include "folly/FBString.h"
#include "folly/FBVector.h"
#include "fmt/format.h"

#include <cereal/types/memory.hpp>
#include <cereal/archives/binary.hpp>
#include <unordered_set>

#include "cereal/cereal.hpp"

namespace cereal
{
	using CharT = char;
	
  //! Serialization for basic_string types, if binary data is supported
	template<typename Archive, typename CharT, typename Allocator> inline
  typename std::enable_if<traits::is_output_serializable<BinaryData<char>, Archive>::value, void>::type
	CEREAL_SAVE_FUNCTION_NAME(Archive & ar, folly::basic_fbstring<CharT, Allocator> const & str)
  {
    // Save number of chars + the data
    ar( make_size_tag( static_cast<size_type>(str.size()) ) );
    ar( binary_data( str.data(), str.size() * sizeof(CharT) ) );
  }

  //! Serialization for basic_string types, if binary data is supported
	template<class Archive, typename CharT, typename Allocator> inline
  typename std::enable_if<traits::is_input_serializable<BinaryData<char>, Archive>::value, void>::type
	CEREAL_LOAD_FUNCTION_NAME(Archive & ar, folly::basic_fbstring<CharT, Allocator> & str)
  {
    size_type size;

	ar( make_size_tag( size ) );
    str.resize(static_cast<std::size_t>(size));
    ar( binary_data( const_cast<CharT *>( str.data() ), static_cast<std::size_t>(size) * sizeof(CharT) ) );
  }
} // namespace cereal


#include "celero/Celero.h"

constexpr int NumberOfSamples = 200000;
constexpr int NumberOfIterations = 10;

std::string aLine("fdsf ggsssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssfdgfd  gfdg f gfdgdf gd ");
folly::fbstring aLinefb(aLine);

const char pattern[] = {"Hello fdsf dsd"};

std::string stdpattern(pattern);
folly::fbstring fbpattern(pattern); 

template<typename String>
bool search(const String &str, const String &pattern) {
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
