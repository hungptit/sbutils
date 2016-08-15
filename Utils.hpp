#ifndef Utils_hpp_
#define Utils_hpp_

#include <iomanip>
#include <iostream>
#include <string>
#include <tuple>

#include "DataStructures.hpp"
#include "Timer.hpp"
#include "boost/algorithm/searching/knuth_morris_pratt.hpp"

namespace utils {
    // Define the file information which is (path, stem, extension, permission,
    // time_stamp, file_size)

    template <typename T> std::string to_fixed_string(const int len, T val) {
        std::ostringstream ss;
        ss << std::setw(len) << std::setfill('0') << val;
        return ss.str();
    }

    class KnuthMorrisPrattFilter {
      public:
        using iter_type = std::string::const_iterator;
        explicit KnuthMorrisPrattFilter(const std::string &pattern)
            : SearchAlg(pattern.begin(), pattern.end()) {}

        bool isValid(const FileInfo &info) { return SearchAlg(info.Path) != info.Path.end(); }

      private:
        boost::algorithm::knuth_morris_pratt<iter_type> SearchAlg;
    };

    template <typename T, typename... Args> bool isValid(const FileInfo &info, const T &first) {
        return first.isValid(info);
    }
    
    template <typename Container> class ExtFilter {
      public:
        explicit ExtFilter(Container &exts) : Extensions(exts) {}

        bool isValid(const FileInfo &info) const {
            if (Extensions.empty()) {
                return true;
            }
            return (std::find(Extensions.begin(), Extensions.end(), info.Extension) !=
                    Extensions.end());
        }

      private:
        Container Extensions;
    };

    template <typename Container> class StemFilter {
      public:
        explicit StemFilter(Container &stems) : Stems(stems) {}

        bool isValid(const FileInfo &info) const {
            if (Stems.empty()) {
                return true;
            }
            return (std::find(Stems.begin(), Stems.end(), info.Stem) != Stems.end());
        }

      private:
        std::vector<std::string> Stems;
    };

    class SimpleFilter {
      public:
        using iter_type = std::string::const_iterator;
        explicit SimpleFilter(const std::string &pattern) : Pattern(pattern) {}

        bool isValid(const FileInfo &info) const {
            return info.Path.find(Pattern) != std::string::npos;
        }

      private:
        std::string Pattern;
    };

    template <typename T, typename... Args>
    bool isValid(const FileInfo &info, const T &first, const Args... args) {
        return first.isValid(info) && isValid(info, args...);
    }

    // Do a simple copy if there is not any constraint.
    template <typename Iterator>
    std::vector<utils::FileInfo> filter(Iterator begin, Iterator end) {
        return std::vector<utils::FileInfo>(begin, end);
    }

    template <typename Iterator, typename FirstConstraint, typename... Constraints>
    std::vector<utils::FileInfo> filter(Iterator begin, Iterator end, FirstConstraint &f1,
                                        Constraints... fs) {
        utils::ElapsedTime<utils::MILLISECOND> t1("Filtering files: ");
        std::vector<utils::FileInfo> results;
        auto filterObj = [f1, &fs..., &results](const auto &item) {
            if (isValid(item, f1, fs...)) {
                results.emplace_back(item);
            }
        };

        // TODO: Speed up this for loop using thread.
        std::for_each(begin, end, filterObj);
        return results;
    }

    template <typename Iterator, typename FirstConstraint, typename SecondConstraint,
              typename ThirdConstraint>
    std::vector<utils::FileInfo>
    filter_nopack(Iterator begin, Iterator end, const FirstConstraint &f1,
                  const SecondConstraint &f2, const ThirdConstraint &f3) {
        utils::ElapsedTime<utils::MILLISECOND> t1("Filtering files: ");
        std::vector<utils::FileInfo> results;
        auto filterObj = [&](const auto &item) {
            if (f1.isValid(item) && f2.isValid(item) && f3.isValid(item)) {
                results.emplace_back(item);
            }
        };

        // TODO: Speed up this for loop using thread.
        std::for_each(begin, end, filterObj);

        return results;
    }
}

#endif
