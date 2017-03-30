#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"
#include <array>
#include <fstream>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

#include "fmt/format.h"
#include "sbutils/FileUtils.hpp"
#include "sbutils/Process.hpp"
#include "sbutils/Timer.hpp"

#include "tbb/parallel_invoke.h"
#include "tbb/tbb.h"

#include "cereal/archives/binary.hpp"
#include "cereal/archives/json.hpp"
#include "cereal/archives/portable_binary.hpp"
#include "cereal/archives/xml.hpp"
#include "cereal/types/array.hpp"
#include "cereal/types/chrono.hpp"
#include "cereal/types/deque.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/vector.hpp"

#include "spdlog/spdlog.h"

auto console = spdlog::stdout_color_mt("console");

namespace {
    struct TestData {
        TestData() = default;

        TestData(const std::string &testName, const int errcode, const std::string &outputLog,
                 const std::string &errorLog)
            : UnitTest(testName), ErrorCode(errcode), OutputLog(outputLog),
              ErrorLog(errorLog){};

        std::string UnitTest;
        int ErrorCode;
        std::string OutputLog;
        std::string ErrorLog;

        template <typename Archive> void serialize(Archive &ar) {
            ar(cereal::make_nvp("Unit test", UnitTest),
               cereal::make_nvp("Error code", ErrorCode),
               cereal::make_nvp("Test log", OutputLog),
               cereal::make_nvp("Error message", ErrorLog));
        }
    };

    template <typename OutputArchive>
    void writeToFile(
        const tbb::concurrent_vector<std::tuple<std::string, sbutils::CommandOutput>> &results,
        const std::string &outputFile) {
        std::vector<TestData> data(results.size());

        // Transform the tuple to struct so that we can have more descriptive output.
        std::transform(results.begin(), results.end(), data.begin(), [](auto item) {
            auto value = std::get<1>(item);
            return TestData(std::get<0>(item), std::get<2>(value), std::get<0>(value),
                            std::get<1>(value));
        });

        // Output to file.
        std::stringstream ss;
        {
            OutputArchive archive(ss);
            archive(cereal::make_nvp("Test data", data));
        }

        std::ofstream ofs;
        ofs.open(outputFile, std::ofstream::out);
        ofs << ss.str();
        ofs.close();

		// Log to the console
		console->info("Write all test results to \"{0}\"", outputFile);
    }

	
	
	auto parseTestLog(const std::string &output) {
		std::tuple<bool, int, int, int> results;
		
		return results;
	}
	
}

int main(int argc, char *argv[]) {
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    std::vector<std::string> unitTests;

    bool nodbh = true;
    bool failedOnly = true;
    bool nocoverage = true;

    std::string outputFile;
	const std::string defaultOutputFile("output.json");
	
    // clang-format off
    desc.add_options()
        ("help,h", "Print this help")
        ("verbose,v", "Display verbose information.")
		("nocoverage", po::value<bool>(&nocoverage)->default_value(true), "Run code coverage.")
		("nodbh", po::value<bool>(&nodbh)->default_value(true), "Do not use database")
		("failedonly", po::value<bool>(&failedOnly)->default_value(true), "only dislay failed tests")
		("output-file,o", po::value<std::string>(&outputFile)->default_value(defaultOutputFile), "Output file name")
		("tests,t", po::value<std::vector<std::string>>(&unitTests), "Test modules.");
    // clang-format on

    po::positional_options_description p;
    p.add("tests", -1);
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc;
        return EXIT_SUCCESS;
    }

    if (unitTests.empty()) {
        console->critical("You need to provide a test module or test point!\n");
        return EXIT_SUCCESS;
    }

    std::vector<std::string> utestArgs;
    bool verbose = vm.count("verbose");

    if (nocoverage) {
        utestArgs.push_back("--nocoverage");
    }

    if (nodbh) {
        utestArgs.push_back("--nodbh");
    }

    if (failedOnly) {
        utestArgs.push_back("--failedonly");
    }

    console->info("Start to run tests!");

    fmt::MemoryWriter writer;
    tbb::concurrent_vector<std::tuple<std::string, sbutils::CommandOutput>> results;
    auto testObj = [&results, &utestArgs, &writer, &unitTests](const int index) {
        sbutils::Timer timer;
        auto newArgs(utestArgs);
        newArgs.push_back(unitTests[index]);
        sbutils::CommandInfo args = std::make_tuple("utest", newArgs);
        sbutils::CommandOutput outputs = sbutils::run(args, "./");
        results.push_back(std::make_tuple(unitTests[index], outputs));
        console->info("Finish {0} in {1} seconds.", unitTests[index],
                      timer.toc() / timer.ticksPerSecond());

		// Parse the test results
		
    };

    int size = static_cast<int>(unitTests.size());
    tbb::parallel_for(0, size, 1, testObj);

    // Write results to an output file
    writeToFile<cereal::JSONOutputArchive>(results, outputFile);

    return EXIT_SUCCESS;
}
