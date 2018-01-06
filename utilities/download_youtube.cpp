// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <cstdlib>
#include <future>
#include <iostream>
#include <string>
#include <vector>

#include "boost/process.hpp"
#include "boost/program_options.hpp"
#include "fmt/format.h"
#include "spdlog/spdlog.h"

namespace {
    struct DownloadInfo {
        std::vector<std::string> links;
        std::string destination_folder;
        bool extract_audio;
    };

    struct CommandOutput {
        std::string stdout;
        std::string stderr;
        int exit_code;
    };

    auto exec(const std::string &cmd) {
		return boost::process::system(cmd.c_str());
    }

    spdlog::level::level_enum get_logging_level(const std::string &loglevel) {
        const std::unordered_map<std::string, spdlog::level::level_enum> lookupTable{
            {"info", spdlog::level::info},   {"debug", spdlog::level::debug},
            {"trace", spdlog::level::trace}, {"warn", spdlog::level::warn},
            {"error", spdlog::level::err},   {"critical", spdlog::level::critical},
            {"off", spdlog::level::off}};
        auto const it = lookupTable.find(loglevel);
        if (it == lookupTable.end()) {
            std::string msg = "Invalid logging level value: " + loglevel;
            throw std::runtime_error(msg);
        }
        return it->second;
    }

    void download(const DownloadInfo &info) {
        auto downloadObj = [&info](const std::string &alink) {
            namespace bp = boost::process;
            fmt::MemoryWriter writer;
            writer << "youtube-dl ";
            writer << ((info.extract_audio) ? " -x --audio-format mp3 --audio-quality 9 "
                                            : " --recode-video mkv ");
            writer << alink;
            auto console = spdlog::get("console");
            console->debug("Download link: {}", alink);
            console->trace("Executed command: {}", writer.str());
            auto errcode = exec(writer.str());
			if (errcode) {
				console->error("Cannot download file from {0}, exit_code: {1}", alink, errcode);
			}
			// console->trace("{}", results.stdout);
            console->info("Done: {}", alink);
        };

        //
        std::vector<std::future<void>> tasks;
        for (auto const alink : info.links) {
            tasks.emplace_back(std::async(std::launch::async, downloadObj, alink));
        }
        std::for_each(tasks.begin(), tasks.end(), [](auto &atask) { atask.get(); });
    } // namespace
} // namespace

int main(int argc, char *argv[]) {
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    DownloadInfo args;
    std::string log_level;

    // clang-format off
    desc.add_options()
      ("help,h", "Print this help")
	  ("log-level,d", po::value<std::string>(&log_level)->default_value("info"), "Specify the level of logging")
      ("extract-audio,a", "Search string pattern.")
      ("folder,f", po::value<std::string>(&args.destination_folder), "Search folders.")
      ("links,l", po::value<std::vector<std::string>>(&args.links), "File extensions.");
    // clang-format on

    po::positional_options_description p;
    p.add("links", -1);
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc;
        return EXIT_SUCCESS;
    }

    spdlog::set_level(get_logging_level(log_level));
    args.extract_audio = true;

    // Download all audio/video files from provided links using youtube-dl
    auto console = spdlog::stdout_color_mt("console");
    console->info("Start download youtube links");
    download(args);
}
