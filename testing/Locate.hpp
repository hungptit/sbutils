#include <array>
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#define BOOST_THREAD_VERSION 4
#include "boost/config.hpp"
#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"
#include "boost/thread.hpp"
#include "boost/thread.hpp"
#include "boost/thread/future.hpp"
#include "utils/LevelDBIO.hpp"
#include "utils/Timer.hpp"
#include "utils/Utils.hpp"

#include "cppformat/format.h"

#include <map>
#include <string>
#include <vector>

namespace {
  boost::mutex UpdateResults;

  /**
   * This implementation distributes work related to each key into an
   * asynchronous thread then combine returned results at the end. We
   * did try to read all file information using threads, however, there is
   * no performane gain.
   *
   * @todo: How can we make this code better?
   */
  class ThreadedLocate {
  public:
    typedef std::set<Utils::FileInfo> Set;
    typedef std::vector<Utils::FileInfo> Container;

    ThreadedLocate(const std::string &dataFile) : Reader(dataFile) {}

    ThreadedLocate(const std::string &dataFile,
                   const std::vector<std::string> &stems)
      : Reader(dataFile), Stems(stems) {}
    ThreadedLocate(const std::string &dataFile,
                   const std::vector<std::string> &stems,
                   const std::vector<std::string> &exts)
      : Reader(dataFile), Stems(stems), Extensions(exts) {}
    ThreadedLocate(const std::string &dataFile,
                   const std::vector<std::string> &stems,
                   const std::vector<std::string> &exts,
                   const std::vector<std::string> &folders)
      : Reader(dataFile), Stems(stems), Extensions(exts),
        Folders(folders) {}

    void locate() {
      auto keys = Reader.keys();
      for (auto const &aKey : keys) {
        updateSearchResults(find(aKey));
      }
    }

    void locate_t() { // Threaded locate
      auto keys = Reader.keys();
      for (auto const &aKey : keys) {
        boost::future<Container> aThread =
          boost::async(std::bind(&ThreadedLocate::find, this, aKey));
        aThread.wait();
        updateSearchResults(aThread.get());
      }
    }

    void print(bool verbose = false) {
      Set results;
      results.insert(Results.begin(), Results.end());
      if (verbose) {
        std::for_each(results.begin(), results.end(),
                      [](auto &item) { std::cout << item << "\n"; });
      } else {
        std::for_each(results.begin(), results.end(), [](auto &item) {
            fmt::print("{}\n", std::get<0>(item));
          });
      }
    }

    Container getResults() const { return Results; }

  private:
    Utils::Reader Reader;
    std::vector<std::string> Keys;
    std::vector<std::string> Stems;
    std::vector<std::string> Extensions;
    std::vector<std::string> Folders;
    Container Results;

    Container find(const std::string &aKey) {
      return filter(deserialize(aKey));
    }

    Container filter(const Container &data) {
      Container results;
      for (auto info : data) {
        bool flag = (Stems.empty() ||
                     std::find(Stems.begin(), Stems.end(),
                               std::get<1>(info)) != Stems.end()) &&
          (Extensions.empty() ||
           std::find(Extensions.begin(), Extensions.end(),
                     std::get<2>(info)) != Extensions.end());
        if (flag) {
          results.emplace_back(info);
        }
      }
      return results;
    }

    void updateSearchResults(const std::vector<Utils::FileInfo> &results) {
      boost::unique_lock<boost::mutex> guard(UpdateResults);
      std::move(results.begin(), results.end(),
                std::back_inserter(Results)); // C++11 feature
    }

    Container deserialize(const std::string &aKey) {
      Timer timer;
      std::istringstream is(Reader.read(aKey));
      Container data;
      Utils::load<Utils::IArchive, decltype(data)>(data, is);
      std::cout << "Deserialize time: "
                << timer.toc() / timer.ticksPerSecond() << " seconds"
                << std::endl;
      return data;
    }
  };
}
