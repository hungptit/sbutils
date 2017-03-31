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
    // Test status, number of tests, number of passed test, number of failed
    // tests, and number of skipped tests.
    using TestFinalResults = std::tuple<unsigned int, unsigned int, unsigned int>;

    // (Unit test name, output log, parsed results, test run time)
    using TestResults =
        std::tuple<std::string, sbutils::CommandOutput, TestFinalResults, double>;

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
    void writeToFile(const tbb::concurrent_vector<TestResults> &results,
                     const std::string &outputFile) {
        std::vector<TestData> data(results.size());

        // Transform the tuple to struct so that we can have more descriptive output.
        std::transform(results.begin(), results.end(), data.begin(), [](auto item) {
            auto value = std::get<1>(item);
            return TestData(std::get<0>(item), std::get<2>(value), std::get<0>(value),
                            std::get<1>(value));
        });

        // Serialize data to string
        std::stringstream ss;
        {
            OutputArchive archive(ss);
            archive(cereal::make_nvp("Results", data));
        }

		// Write to an output file.
        std::ofstream ofs;
        ofs.open(outputFile, std::ofstream::out);
        ofs << ss.str();
        ofs.close();

        // Log to the console
        console->info("Write all test results to \"{0}\"", outputFile);
    }

    auto parseTestLog(const std::string &output) {
        TestFinalResults results;

        unsigned int allTests = 101;
        unsigned int passedTests = 99;
        unsigned int failedTests = 1;
        unsigned int skippedTests = 1;

        assert(allTests == (passedTests + failedTests + skippedTests));

        // TODO: Implement this function
        return std::make_tuple(passedTests, failedTests, skippedTests);
    }
}

int main(int argc, char *argv[]) {
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    std::vector<std::string> unitTests;
    bool failedOnly = true;
    std::string outputFile;
    std::string inputFile;
    const std::string defaultOutputFile("output.json");

    // clang-format off
    desc.add_options()
        ("help,h", "Print this help")
        ("verbose,v", "Display verbose information.")
		("coverage", "Run code coverage.")
		("auto", "Find out tests need to run automatically.")
		("nodbh", "Do not use database")
		("failedonly", po::value<bool>(&failedOnly)->default_value(true), "only dislay failed tests")
		("output-file,o", po::value<std::string>(&outputFile)->default_value(defaultOutputFile), "Output file name")
		("input-file,i", po::value<std::string>(&inputFile)->default_value(""), "An input file which has test modules.")
		("test-modules,t", po::value<std::vector<std::string>>(&unitTests), "Test modules.");
    // clang-format on

    po::positional_options_description p;
    p.add("test-modules", -1);
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

    // bool verbose = vm.count("verbose");

    if (!vm.count("coverage")) {
        utestArgs.push_back("--nocoverage");
    }

    if (!vm.count("nodbh")) {
        utestArgs.push_back("--nodbh");
    }

    if (failedOnly) {
        utestArgs.push_back("--failedonly");
    }

    console->info("Start to run tests!");

    fmt::MemoryWriter writer;
    tbb::concurrent_vector<TestResults> results;
    auto testObj = [&results, &utestArgs, &writer, &unitTests](const int index) {
        sbutils::Timer timer;
        auto newArgs(utestArgs);
        newArgs.push_back(unitTests[index]);
        sbutils::CommandInfo args = std::make_tuple("utest", newArgs);
        sbutils::CommandOutput outputs = sbutils::run(args, "./");

        // Parse the test results
        TestFinalResults unitTestResults = parseTestLog(std::get<0>(outputs));
        const double runTime = timer.toc() / timer.ticksPerSecond();

        results.push_back(std::make_tuple(unitTests[index], outputs, unitTestResults, runTime));

        // Log high level information to console.
        if (std::get<1>(unitTestResults) == 0) { // Passed
            console->info("Test results for {0}: {1} passed, {2} failed, {3} skipped, and "
                          "total time is {4} seconds",
                          unitTests[index], std::get<0>(unitTestResults),
                          std::get<1>(unitTestResults), std::get<2>(unitTestResults), runTime);
        } else {
            console->critical(
                "Test results for {0}: {1} passed, {2}, failed {3}, and total time is {4} seconds",
                unitTests[index], std::get<0>(unitTestResults), std::get<1>(unitTestResults),
                std::get<2>(unitTestResults), runTime);
        }

    };

    int size = static_cast<int>(unitTests.size());
    tbb::parallel_for(0, size, 1, testObj);

    // Write results to an output file
    writeToFile<cereal::JSONOutputArchive>(results, outputFile);

    return EXIT_SUCCESS;
}
