#include <iostream>
#include <stdexcept>
#include <fstream>
#include "folly/FBString.h"
#include "fmt/format.h"

#include <cereal/types/memory.hpp>
#include <cereal/archives/binary.hpp>
#include <unordered_set>

#include "cereal/cereal.hpp"

namespace cereal
{
	using CharT = char;
	
  //! Serialization for basic_string types, if binary data is supported
  template<class Archive> inline
  typename std::enable_if<traits::is_output_serializable<BinaryData<char>, Archive>::value, void>::type
  CEREAL_SAVE_FUNCTION_NAME(Archive & ar, folly::fbstring const & str)
  {
    // Save number of chars + the data
    ar( make_size_tag( static_cast<size_type>(str.size()) ) );
    ar( binary_data( str.data(), str.size() * sizeof(CharT) ) );
  }

  //! Serialization for basic_string types, if binary data is supported
  template<class Archive> inline
  typename std::enable_if<traits::is_input_serializable<BinaryData<char>, Archive>::value, void>::type
  CEREAL_LOAD_FUNCTION_NAME(Archive & ar, folly::fbstring & str)
  {
    size_type size;
    ar( make_size_tag( size ) );
    str.resize(static_cast<std::size_t>(size));
    ar( binary_data( const_cast<CharT *>( str.data() ), static_cast<std::size_t>(size) * sizeof(CharT) ) );
  }
} // namespace cereal


int main() {
	folly::fbstring str("Hello");
	fmt::print("{}\n", str.data());
	std::unordered_set<folly::fbstring> dict;

	std::ofstream os("out.cereal", std::ios::binary);
	cereal::BinaryOutputArchive archive( os );
	archive(str);

	
}
