#ifndef Utils_hpp_
#define Utils_hpp_

#include <iomanip>
#include <iostream>
#include <string>
#include <tuple>
#include <utility>

#include "DataStructures.hpp"
#include "Timer.hpp"
#include "boost/algorithm/searching/knuth_morris_pratt.hpp"

#include "tbb/tbb.h"

namespace {
    template <typename T, typename... Args>
    bool isValid(const utils::FileInfo &info, T &&first) {
        return first.isValid(info);
    }
    
    template <typename T, typename... Args>
    bool isValid(const utils::FileInfo &info, T &&first, Args&&... args) {
        return first.isValid(info) && isValid(info, std::forward<Args>(args)...);
    }    
}

namespace utils {
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

    // Do a simple copy if there is not any constraint.
    template <typename Iterator>
    std::vector<utils::FileInfo> filter(Iterator begin, Iterator end) {
        return std::vector<utils::FileInfo>(begin, end);
    }

    template <typename Container, typename FirstConstraint, typename... Constraints>
    auto filter(Container &&data, FirstConstraint &&f1,
                                        Constraints&&... fs) {
        utils::ElapsedTime<utils::MILLISECOND> t1("Filtering files: ");
        std::vector<utils::FileInfo> results;
        auto filterObj = [f1, &fs..., &results](const auto &item) {
            if (isValid(item, f1, std::forward<Constraints>(fs)...)) {
                results.emplace_back(item);
            }
        };

        std::for_each(data.begin(), data.end(), filterObj);
        return results;
    }

  template <typename Container, typename FirstConstraint, typename... Constraints>
  auto filter_tbb(Container &&data, FirstConstraint &&f1,
              Constraints&&... fs) {
        utils::ElapsedTime<utils::MILLISECOND> t1("Filtering files param pack: ");
        tbb::concurrent_vector<utils::FileInfo> results;
        
        auto filterObj = [f1, &fs..., &results, &data](const int idx) {
          auto const &item = data[idx];
            if (isValid(item, f1, std::forward<Constraints>(fs)...)) {
              results.push_back(item);
            }
        };

        int size = data.size();
        tbb::parallel_for(0, size, 1, filterObj);
        return results;
    }
}

#endif
