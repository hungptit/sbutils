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

    template <typename Container> class ExtFilter {
      public:
        explicit ExtFilter(Container &exts) : Extensions(exts) {}

        bool isValid(FileInfo &info) {
            if (Extensions.empty()) {
                return true;
            } else {
                return (std::find(Extensions.begin(), Extensions.end(), info.Extension) !=
                        Extensions.end());
            }
        }

      private:
        Container Extensions;
    };

    template <typename Container> class StemFilter {
      public:
        explicit StemFilter(Container &stems) : Stems(stems) {}

        bool isValid(FileInfo &info) {
            if (Stems.empty()) {
                return true;
            } else {
                return (std::find(Stems.begin(), Stems.end(), info.Stem) != Stems.end());
            }
        }

      private:
        std::vector<std::string> Stems;
    };

    class SimpleFilter {
      public:
        using iter_type = std::string::const_iterator;
        explicit SimpleFilter(const std::string &pattern)
            : Pattern(pattern) {}

        bool isValid(const FileInfo &info) {
          return info.Path.find(Pattern) != std::string::npos;
        }

      private:
        std::string Pattern;
    };

    class KnuthMorrisPrattFilter {
      public:
        using iter_type = std::string::const_iterator;
        explicit KnuthMorrisPrattFilter(const std::string &pattern)
            : SearchAlg(pattern.begin(), pattern.end()) {}

        bool isValid(const FileInfo &info) { return SearchAlg(info.Path) != info.Path.end(); }

      private:
        boost::algorithm::knuth_morris_pratt<iter_type> SearchAlg;
    };

    template <typename Iterator, typename Filter1>
    std::vector<utils::FileInfo> filter(Iterator begin, Iterator end, Filter1 &f1) {
        std::vector<utils::FileInfo> results;
        auto filterObj = [&f1, &results](auto &item) {
            if (f1.isValid(item)) {
                results.emplace_back(item);
            }
        };

        // TODO: Speed up this for loop using thread.
        std::for_each(begin, end, filterObj);

        return results;
    }

    template <typename Iterator, typename Filter1, typename Filter2>
    std::vector<utils::FileInfo> filter(Iterator begin, Iterator end, Filter1 &f1,
                                        Filter2 &f2) {
        std::vector<utils::FileInfo> results;

        auto filterObj = [&f1, &f2, &results](auto &item) {
            if (f1.isValid(item) && f2.isValid(item)) {
                results.emplace_back(item);
            }
        };

        // TODO: Speed up this for loop using thread.
        std::for_each(begin, end, filterObj);

        return results;
    }

    template <typename Iterator, typename Filter1, typename Filter2, typename Filter3>
    std::vector<utils::FileInfo> filter(Iterator begin, Iterator end, Filter1 &f1, Filter2 &f2,
                                        Filter3 &f3) {
        std::vector<utils::FileInfo> results;

        auto filterObj = [&f1, &f2, &f3, &results](auto &item) {
            if (f1.isValid(item) && f2.isValid(item) && f3.isValid(item)) {
                results.emplace_back(item);
            }
        };

        // TODO: Speed up this for loop using thread.
        std::for_each(begin, end, filterObj);

        return results;
    }

    template <typename Container, typename ExtContainer, typename StemContainer>
    auto filterSearchResults(Container &data, ExtContainer &exts, StemContainer &stems) {
        utils::ElapsedTime<utils::MILLISECOND> t1("Filtering files: ");
        utils::ExtFilter<ExtContainer> f1(exts);
        utils::StemFilter<StemContainer> f2(stems);
        return utils::filter(data.begin(), data.end(), f1, f2);
    }

    template <typename Container, typename ExtContainer, typename StemContainer>
    auto filterSearchResults(Container &data, ExtContainer &exts, StemContainer &stems,
                             const std::string &pattern) {
        utils::ElapsedTime<utils::MILLISECOND> t1("Filtering files: ");
        utils::ExtFilter<ExtContainer> f1(exts);
        utils::StemFilter<StemContainer> f2(stems);
        utils::SimpleFilter f3(pattern);
        return utils::filter(data.begin(), data.end(), f1, f2, f3);
    }
}

#endif
