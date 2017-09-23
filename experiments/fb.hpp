#pragma once

#include "folly/FBString.h"
#include "folly/FBVector.h"

#include "cereal/cereal.hpp"
#include <cereal/archives/binary.hpp>
#include <cereal/types/memory.hpp>

namespace cereal {
    template <typename Archive, typename CharT, typename Allocator>
    inline typename std::enable_if<
        traits::is_output_serializable<BinaryData<char>, Archive>::value, void>::type
    CEREAL_SAVE_FUNCTION_NAME(Archive &ar, folly::basic_fbstring<CharT, Allocator> const &str) {
        // Save number of chars + the data
        ar(make_size_tag(static_cast<size_type>(str.size())));
        ar(binary_data(str.data(), str.size() * sizeof(CharT)));
    }

    //! Serialization for basic_string types, if binary data is supported
    template <class Archive, typename CharT, typename Allocator>
    inline
        typename std::enable_if<traits::is_input_serializable<BinaryData<char>, Archive>::value,
                                void>::type
        CEREAL_LOAD_FUNCTION_NAME(Archive &ar, folly::basic_fbstring<CharT, Allocator> &str) {
        size_type size;

        ar(make_size_tag(size));
        str.resize(static_cast<std::size_t>(size));
        ar(binary_data(const_cast<CharT *>(str.data()),
                       static_cast<std::size_t>(size) * sizeof(CharT)));
    }
} // namespace cereal
