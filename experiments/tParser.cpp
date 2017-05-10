#include <iostream>
#include <vector>

#include "fmt/format.h"
#include "sbutils/FileUtils.hpp"

#include "boost/unordered_set.hpp"

namespace {
    template <typename Container> void print(Container &&edges) {
        std::for_each(edges.cbegin(), edges.cend(), [](auto &item) {
            fmt::print("{} -> {}\n", std::get<0>(item), std::get<1>(item));
        });
    }
} // namespace

namespace perlmoddep {
    constexpr char eol('\n'), delim('+'), star('*');

    template <typename Iter> std::string getNodeName(Iter begin, Iter end) {
        Iter prevIter = end - 1;
        if ((*prevIter) == star) {
            return std::string(begin, prevIter);
        } else {
            return std::string(begin, end);
        }
    }

    template <typename Iter> auto parser(Iter begin, Iter end) {
        using edge_type = std::tuple<std::string, std::string>;
        boost::unordered_set<edge_type> edges;

        std::string parentNode, currentNode, previousNode;
        int currentNodeHeight = 0, previousNodeHeight = 0, parentNodeHeight = 0;
        Iter prevIter = begin, iter = prevIter;

        // Get the height of the first node
        for (; (iter != end) && ((*iter) == delim); ++iter) {
        }

        previousNodeHeight = std::distance(prevIter, iter);
        parentNodeHeight = previousNodeHeight;
        prevIter = iter;

        // Get the name of the root node
        for (; (iter != end) && ((*iter) != eol); ++iter) {
        }
        parentNode = getNodeName(prevIter, iter);
        previousNode = parentNode;

        // Move to the next line
        ++iter;
        prevIter = iter;

        std::vector<std::tuple<int, std::string>> stack;
        while (iter != end) {
            // Get the current node height by detect '+' character.
            for (; (iter != end) && ((*iter) == delim); ++iter) {
            }

            currentNodeHeight = std::distance(prevIter, iter);
            prevIter = iter;

            // get the name of the root node
            for (; (iter != end) && ((*iter) != eol); ++iter) {
            }
            currentNode = getNodeName(prevIter, iter);

            // Get current edge information
            int steps = currentNodeHeight - previousNodeHeight;
            if (steps == 0) {
                edges.emplace(std::make_tuple(parentNode, currentNode));
            } else if (steps == 1) {
                edges.emplace(std::make_tuple(previousNode, currentNode));
                stack.push_back(std::make_tuple(parentNodeHeight, parentNode));
                parentNode = previousNode;
                parentNodeHeight = previousNodeHeight;
            } else if (steps < 0) {
                int N(-steps - 1);
                for (int idx = 0; idx < N; ++idx) {
                    stack.pop_back();
                }
                std::tie(parentNodeHeight, parentNode) = stack.back();
                stack.pop_back();

                // The current node might be at the lower level than that of
                // the parent node.
                if (currentNodeHeight > parentNodeHeight) {
                    edges.emplace(std::make_tuple(parentNode, currentNode));
                }
            } else {
                fmt::MemoryWriter writer;
                writer
                    << "parentNode: " << parentNode << "\n"
                    << "previousNode: " << previousNode << "\n"
                    << "currentNode: " << currentNode << "\n"
                    << "The data is ill-formatted. We cann't parse the data!";
                throw(std::runtime_error(writer.str()));
            }

            // Get out of this loop if we have reached the end of the given
            // range.
            if (iter == end) break;

            // Move to the next line
            ++iter;
            prevIter = iter;
            previousNode = currentNode;
            previousNodeHeight = currentNodeHeight;
        }

        // Return a unique list of edges.
        return std::vector<edge_type>(edges.begin(), edges.end());
    }

} // namespace perlmoddep

namespace junit {
    enum STATUS { OK, FAILED, WIP, SKIPPED };
    struct TestPoint {
        STATUS Status;
        unsigned int Index;
        std::string Name;
        std::string Body;
    };

  constexpr char eolChar('\n'), tabChar('\t'), spaceChar(' '); 
  
    template <typename Iter> auto parseLog(Iter begin, Iter end) {

      Iter prevIter(begin), currentIter(begin), emptyLineIter(begin);
      std::string aLine;
      std::vector<size_t> lineSizes;
      std::vector<Iter> emptyLines;

      fmt::MemoryWriter writer;
      
      while (currentIter != end) {
        // Read the current line
        for (; (currentIter != end) && (*currentIter != eolChar); ++currentIter) {
        }
        
        size_t len = std::distance(prevIter, currentIter);
        if (len == 0) {         
          writer << "--- Start Block ---\n" << std::string(emptyLineIter, currentIter) << "--- Stop block ---\n";

          // Skip next empty lines
          for (; (currentIter != end) && (*currentIter == eolChar); ++currentIter) {
          }
          emptyLineIter = currentIter;
        } else {
          // fmt::print("{0} -> {1}\n", std::string(prevIter, currentIter), len);
        }

        // Move the next line if possible
        if (currentIter == end) {
          break;
        } else {
          ++currentIter;
          prevIter = currentIter;
        }
      }

      fmt::print("{}\n", writer.str());
        // Package %s: Running unit tests for # subroutines.
    }

    void toJUnitXML() {}
} // namespace junit

int main() {
    // {
    //     const std::string dataFile("dep2.log");
    //     std::string data = sbutils::read(dataFile);
    //     auto edges = perlmoddep::parser(data.cbegin(), data.cend());
    //     print(edges);
    // }

    {
        const std::string dataFile("WorkUnit--Utils.log");
        std::string data = sbutils::read(dataFile);
        junit::parseLog(data.cbegin(),data.cend());
        
        
    }
}
