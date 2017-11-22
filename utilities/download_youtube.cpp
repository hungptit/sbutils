// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "boost/program_options.hpp"
#include <cstdlib>
#include <future>
#include <string>
#include <vector>
#include <iostream>

#include "fmt/format.h"
#include "spdlog/spdlog.h"

namespace {
    struct DownloadInfo {
        std::vector<std::string> links;
        std::string destination_folder;
        std::string log_level;
        bool extract_audio;
    };

    void download(const DownloadInfo &info) {
      auto downloadObj = [&info](const std::string &alink) {
            fmt::MemoryWriter writer;
            writer << "youtube-dl ";
            writer << ((info.extract_audio) ? " -x --audio-format mp3 --audio-quality 9 "
                       : " --recode-video mkv ");
            writer << alink;
            spdlog::get("console")->info("Executed command: {}", writer.str());
            std::system(writer.str().c_str());
        };

      std::for_each(info.links.cbegin(), info.links.cend(), downloadObj);
      
    } // namespace
} // namespace

int main(int argc, char *argv[]) {
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    DownloadInfo args;
    
    // clang-format off
    desc.add_options()
      ("help,h", "Print this help")
      ("log-level,d", po::value<std::string>(&args.log_level), "Specify the level of logging")
      ("extract-audio,a", "Search string pattern.")
      ("folder,f", po::value<std::string>(&args.destination_folder), "Search folders.")
      ("links,l", po::value<std::vector<std::string>>(&args.links), "File extensions.");
    // clang-format on

    po::positional_options_description p;
    p.add("pattern", -1);
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc;
        return EXIT_SUCCESS;
    }
    
    // Download all audio/video files from provided links using youtube-dl
    auto console = spdlog::stdout_color_mt("console");
    console->info("Start download youtube links");
    download(args);
}
