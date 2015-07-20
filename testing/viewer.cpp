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
    class ThreadedLocate {
      public:
        typedef std::set<Tools::EditedFileInfo> Set;
        ThreadedLocate(const std::string &dataFile) : Reader(dataFile) {}
        ThreadedLocate(const std::string &dataFile, const std::vector<std::string> &stems)
            : Reader(dataFile), Stems(stems) {}
        ThreadedLocate(const std::string &dataFile, const std::vector<std::string> &stems,
                       const std::vector<std::string> &exts)
            : Reader(dataFile), Stems(stems), Extensions(exts) {}
        ThreadedLocate(const std::string &dataFile, const std::vector<std::string> &stems,
                       const std::vector<std::string> &exts, const std::vector<std::string> &searchStrings)
            : Reader(dataFile), Stems(stems), Extensions(exts), SearchStrings(searchStrings) {}

        // TODO: Threaded this function.
        void locate() {
            auto keys = Reader.keys();
            for (auto const &aKey : keys) {
                auto results = find(aKey);
                for (auto item : results) {
                    Results.insert(item);
                }
            }
        }

        std::vector<Tools::EditedFileInfo> find(const std::string &aKey) {
            std::vector<Tools::EditedFileInfo> data;
            std::vector<Tools::EditedFileInfo> results;
            std::istringstream is(Reader.read(aKey));
            Tools::load<Tools::DefaultIArchive, decltype(data)>(data, is);
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

        void print() {
            for (auto const &item : Results)
                std::cout << std::get<0>(item) << "\n";
        }

      private:
        Tools::Reader Reader;
        std::vector<std::string> Keys;
        std::vector<std::string> Stems;
        std::vector<std::string> Extensions;
        std::vector<std::string> SearchStrings;
        Set Results;
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
        locateObj.locate();
        locateObj.print();
    }

    return 0;
}
