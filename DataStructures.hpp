#ifndef DataStructures_hpp_
#define DataStructures_hpp_

#include <string>
#include <utility>
#include <vector>

#include "cereal/archives/binary.hpp"
#include "cereal/archives/json.hpp"
#include "cereal/archives/portable_binary.hpp"
#include "cereal/archives/xml.hpp"
#include "cereal/types/array.hpp"
#include "cereal/types/chrono.hpp"
#include "cereal/types/deque.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/tuple.hpp"
#include "cereal/types/vector.hpp"

namespace utils {
    template <typename String = std::string> struct NewFileInfo {
        NewFileInfo()
            : Permissions(), Size(), Path(), Stem(), Extension(), TimeStamp() {}

        template <typename T1, typename T2, typename T3, typename T4,
                  typename T5, typename T6>
        NewFileInfo(T1 &&perms, T2 &&sizes, T3 &&path, T4 &&stem, T5 &&ext,
                    T6 &&timeStamp) noexcept
            : Permissions(std::forward<T1>(perms)),
              Size(std::forward<T2>(sizes)), Path(std::forward<T3>(path)),
              Stem(std::forward<T4>(stem)), Extension(std::forward<T5>(ext)),
              TimeStamp(std::forward<T6>(timeStamp))  {}

        NewFileInfo(const NewFileInfo &info) noexcept
            : Permissions(info.Permissions), Size(info.Size), Path(info.Path),
              Stem(info.Stem), Extension(info.Extension),
              TimeStamp(info.TimeStamp) {}

        NewFileInfo(NewFileInfo &&info) noexcept
            : Permissions(info.Permissions), Size(info.Size), Path(std::move(info.Path)),
              Stem(info.Stem), Extension(info.Extension),
              TimeStamp(info.TimeStamp) {}

        template <typename Archive> void serialize(Archive &ar) {
            ar(cereal::make_nvp("perms", Permissions),
               cereal::make_nvp("size", Size), cereal::make_nvp("path", Path),
               cereal::make_nvp("stem", Stem),
               cereal::make_nvp("ext", Extension),
               cereal::make_nvp("time_stamp", TimeStamp));
        }

        // Data members
        int Permissions;
        uintmax_t Size;
        String Path;
        String Stem;
        String Extension;
        std::time_t TimeStamp;
    };

    template <typename String>
    bool operator==(const NewFileInfo<String> &lhs,
                    const NewFileInfo<String> &rhs) {
        return (lhs.Size == rhs.Size) && (lhs.Path == rhs.Path);
    }
}

namespace std {
    // We only care about the size and path of files.
    template <typename String> struct hash<utils::NewFileInfo<String>> {
        using value_type = utils::NewFileInfo<String>;
        typedef std::size_t result_type;

        result_type operator()(const value_type &aKey) const {
            result_type const h1(std::hash<uintmax_t>()(aKey.Size));
            result_type const h2(std::hash<String>()(aKey.Path));
            return h1 ^ (h2 << 1);
        }
    };
}

#endif
