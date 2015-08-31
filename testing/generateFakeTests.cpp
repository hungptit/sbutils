#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <tuple>
#include "boost/filesystem.hpp"
#include "utils/Utils.hpp"
#include "utils/FindUtils.hpp"
#include "utils/LevelDBIO.hpp"
#include "boost/program_options.hpp"
#include "cppformat/format.h"

#include <string>
#include <vector>
#include <map>
#include <ctime>
#include <sstream>

class GenerateFakeTests {
  public:
    typedef boost::filesystem::path Path;
    GenerateFakeTests(const Path &outputFolder, const Path &dataFolder)
        : OutputFolder(outputFolder), DataFolder(dataFolder) {}

    template <typename Container> size_t generate(Container &data) {
        size_t counter = 0;
        auto start = data.begin();
        auto stop = start;
        while (true) {
            std::advance(stop, NumModelPerTest);
            auto distance = std::distance(start, data.end());
            if (distance > NumModelPerTest) {
                generateTest(start, stop, counter);
                std::cout << std::distance(start, stop) << "\n";
                start = stop;
                counter++;
            } else {
                generateTest(start, data.end(), counter);
                return counter++;
            }
        }
        return counter;
    }

  private:
    template <typename Iterator>
    size_t generateTest(Iterator begin, Iterator end, const size_t counter) {
        fmt::MemoryWriter writer;
        auto className = "t" + std::to_string(counter);
        writer << "classdef " << className << " < matlab.unittest.TestCase\n";
        writer << "methods (Test)\n";

        size_t function_counter = 0;
        for (auto iter = begin; iter != end; ++iter, ++function_counter) {
            auto val = *iter;
            writer << "function "
                   << "test_model_" << function_counter << "(this)\n";
            auto aPath = std::get<0>(val);
            auto modelName = std::get<1>(val);
            writer << "aPath = fullfile(matlabroot, '../','" << std::get<0>(val)
                   << "');\n";
            writer << "dataPath = '" << DataFolder.string() << "';\n";
            writer << "tag = '" << className << "';\n";
            writer << "addpath(aPath);\n";
            writer << "modelName = '" << modelName << "';\n";
            writer << "testObj = modelreference.UnitTestSetup([], modelName, "
                      "@load_system);\n";
            writer << "modelreference.FindSubsystems.run(aPath, modelName, "
                      "tag, dataPath);\n";
            writer << "rmpath\n";
            writer << "end\n";
        }

        writer << "end\n";
        writer << "end\n";

        // fmt::print("{}\n", writer.str());
        auto testFile = OutputFolder / Path(className + ".m");
        std::ofstream myfile(testFile.string());
        myfile << writer.str() << std::endl;
        return function_counter;
    }

    boost::filesystem::path OutputFolder;
    boost::filesystem::path DataFolder;
    int NumModelPerTest = 20;
};

int main(int argc, char *argv[]) {
    typedef cereal::JSONOutputArchive OArchive;
    typedef cereal::JSONInputArchive IArchive;

    using namespace boost;
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");

    // clang-format off
    desc.add_options()              
        ("help,h", "Print this help")
        ("verbose,v", "Display searched data.")
        ("output-folder,o", po::value<std::string>(), "Store the fake tests.")
        ("folders,f", po::value<std::vector<std::string>>(), "Search folders.")
        ("extensions,e", po::value<std::vector<std::string>>(), "File extensions.")
        ("data-folder,d", po::value<std::string>(), "Store all collected data.");
    // clang-format on

    po::positional_options_description p;
    p.add("folders", -1);
    po::variables_map vm;
    po::store(
        po::command_line_parser(argc, argv).options(desc).positional(p).run(),
        vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc;
        std::cout << "Examples:" << std::endl;
        std::cout << "\t findEditedFiles ./ -d .database" << std::endl;
        return EXIT_SUCCESS;
    }

    auto verbose = false;
    if (vm.count("verbose")) {
        verbose = true;
    }

    std::string outputFolderStr;
    if (vm.count("output-folder")) {
        outputFolderStr = vm["output-folder"].as<std::string>();
    }

    // Notes: All folder paths must be full paths.
    boost::system::error_code errcode;
    std::vector<std::string> folders;
    if (vm.count("folders")) {
        for (auto item : vm["folders"].as<std::vector<std::string>>()) {
            folders.emplace_back(boost::filesystem::path(item).string());
        }
    }

    std::vector<std::string> stems;
    if (vm.count("file-stems")) {
        stems = vm["file-stems"].as<std::vector<std::string>>();
    }

    std::vector<std::string> extensions;
    if (vm.count("extensions")) {
        extensions = vm["extensions"].as<std::vector<std::string>>();
    }

    std::string dataFolderStr;
    if (vm.count("data-folder")) {
        dataFolderStr = vm["data-folder"].as<std::string>();
    }

    Tools::FindFiles<Tools::Finder> searchAlg(extensions);
    for (auto &val : folders) {
        searchAlg.search(val);
    }
    auto data = searchAlg.getData();

    // Display search folder and the number of models if required.
    if (verbose) {
        fmt::print("Search folders:\n");
        for (const auto &val : folders) {
            fmt::print("{}\n", val);
        }
        fmt::print("Number of files: {}\n", data.size());
        fmt::print("Output folder: {}\n", outputFolderStr);
        fmt::print("Data folder: {}\n", dataFolderStr);
    }

    // Create output folders if neccesary.
    boost::filesystem::path outputFolder(outputFolderStr);
    if (!boost::filesystem::is_directory(outputFolder)) {
        boost::filesystem::create_directories(outputFolder);
    }

    boost::filesystem::path dataFolder(dataFolderStr);
    if (!boost::filesystem::is_directory(dataFolder)) {
        boost::filesystem::create_directories(dataFolder);
    }

    // Generate the fake tests which can be executed by sbruntests.
    GenerateFakeTests codegen(boost::filesystem::canonical(outputFolder),
                              boost::filesystem::canonical(dataFolder));
    std::cout << "Number of generated tests: " << codegen.generate(data)
              << std::endl;

    // Write model information to a JSON file.
    auto jsonFile = dataFolder / boost::filesystem::path("model.json");
    if (!jsonFile.empty()) {
        std::ostringstream os;
        Tools::save<OArchive, decltype(data)>(data, os);
        std::ofstream myfile(jsonFile.string());
        myfile << os.str() << std::endl;
    }

    return 0;
}
