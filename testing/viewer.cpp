#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

#define BOOST_THREAD_VERSION 4
#include "boost/config.hpp"
#include "boost/thread.hpp"
#include "boost/thread/future.hpp"
#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"
#include "boost/thread.hpp"

#include "utils/Utils.hpp"
#include "utils/FindUtils.hpp"
#include "utils/LevelDBIO.hpp"

#include "InputArgumentParser.hpp"

#include <string>
#include <vector>
#include <map>
#include <ctime>
#include <sstream>

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
        typedef std::set<Tools::EditedFileInfo> Set;
        typedef std::vector<Tools::EditedFileInfo> Container;

        ThreadedLocate(const std::string &dataFile) : Reader(dataFile) {}

        ThreadedLocate(const std::string &dataFile, const std::vector<std::string> &stems)
            : Reader(dataFile), Stems(stems) {}
        ThreadedLocate(const std::string &dataFile, const std::vector<std::string> &stems,
                       const std::vector<std::string> &exts)
            : Reader(dataFile), Stems(stems), Extensions(exts) {}
        ThreadedLocate(const std::string &dataFile, const std::vector<std::string> &stems,
                       const std::vector<std::string> &exts, const std::vector<std::string> &searchStrings)
            : Reader(dataFile), Stems(stems), Extensions(exts), SearchStrings(searchStrings) {}

        void locate() {
            auto keys = Reader.keys();
            for (auto const &aKey : keys) {
                update(find(aKey));
            }
        }

        void locate_t() {
            auto keys = Reader.keys();
            for (auto const &aKey : keys) {
                boost::future<Container> aThread =
                    boost::async(std::bind(&ThreadedLocate::find, this, aKey));
                aThread.wait(); // TODO: Do we need to call wait?
                update(aThread.get());
            }
        }

        Container find(const std::string &aKey) {
            auto const data = read(aKey);
            return filter(data);
        }

        void print(bool verbose = false) {
            Set sortedResults;
            sortedResults.insert(Results.begin(), Results.end());
            if (verbose) {
                for (auto const &item : sortedResults)
                    std::cout << item << "\n";
            } else {
                for (auto const &item : sortedResults)
                    std::cout << std::get<0>(item) << "\n";
            }
        }

        Container getResults() const { return Results; }

      private:
        Tools::Reader Reader;
        std::vector<std::string> Keys;
        std::vector<std::string> Stems;
        std::vector<std::string> Extensions;
        std::vector<std::string> SearchStrings;
        Container Results;

        // TODO: Improve this function
        Container filter(const Container & data) {
            Container results;
            for (auto info : data) {
                bool flag = (Stems.empty() ||
                             std::find(Stems.begin(), Stems.end(), std::get<1>(info)) != Stems.end()) &&
                    (Extensions.empty() ||
                     std::find(Extensions.begin(), Extensions.end(), std::get<2>(info)) !=
                     Extensions.end()) &&
                    (SearchStrings.empty() || true);
                if (flag) {
                    results.emplace_back(info);
                }
            }
            return results;
        }

        void update(const std::vector<Tools::EditedFileInfo> &results) {
            boost::unique_lock<boost::mutex> guard(UpdateResults);
            std::move(results.begin(), results.end(), std::back_inserter(Results)); // C++11 feature
        }

        Container read(const std::string &aKey) {
            Container data;
            std::istringstream is(Reader.read(aKey));
            Tools::load<Tools::DefaultIArchive, decltype(data)>(data, is);
            return data;
        }
    };
}

int main(int argc, char *argv[]) {
    using namespace boost;
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");

    // clang-format off
  desc.add_options()
    ("help,h", "Print this help")
    ("verbose,v", "Display searched data.")
    ("parallel,p", "Use threaded version of viewer.")
    ("keys,k", "List all keys.")
    ("folders,f", po::value<std::vector<std::string>>(), "Search folders.")
    ("stems,s", po::value<std::vector<std::string>>(), "File stems.")
    ("extensions,e", po::value<std::vector<std::string>>(), "File extensions.")
    ("strings,t", po::value<std::vector<std::string>>(), "Search string")
    ("database,d", po::value<std::string>(), "File database.");
    // clang-format on

    po::positional_options_description p;
    p.add("database", -1);
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << "Usage: buildEditedFileDatabase [options]\n";
        std::cout << desc;
        std::cout << "Examples:" << std::endl;
        std::cout << "\t finder ./ -d testdata." << std::endl;
        return 0;
    }

    bool verbose = false;
    if (vm.count("verbose")) {
        verbose = true;
    }

    bool isThreaded = false;
    if (vm.count("parallel")) {
        isThreaded = true;
    }

    // Search folders
    std::vector<std::string> folders;
    if (vm.count("folders")) {
        folders = vm["folders"].as<std::vector<std::string>>();
    }

    // Get file stems
    std::vector<std::string> stems;
    if (vm.count("stems")) {
        stems = vm["stems"].as<std::vector<std::string>>();
    }

    // Get file extensions
    std::vector<std::string> extensions;
    if (vm.count("extensions")) {
        extensions = vm["extensions"].as<std::vector<std::string>>();
    }

    // Get file extensions
    std::vector<std::string> searchStrings;
    if (vm.count("strings")) {
        searchStrings = vm["strings"].as<std::vector<std::string>>();
    }

    // Get file database
    std::string dataFile;
    if (vm.count("database")) {
        dataFile = vm["database"].as<std::string>();
    } else {
        dataFile = (boost::filesystem::path(Tools::FileDatabaseInfo::Database)).string();
    }

    if (verbose) {
        std::cout << "Database: " << dataFile << std::endl;
    }

    if (vm.count("keys")) {
        Tools::Reader reader(dataFile);
        for (auto &aKey : reader.keys()) {
            std::cout << aKey << std::endl;
        }
    } else {
        auto locateObj = ThreadedLocate(dataFile, stems, extensions, searchStrings);
        if (isThreaded) {
            locateObj.locate();
        } else {
            locateObj.locate_t();
        }
        locateObj.print();
    }

    return 0;
}
