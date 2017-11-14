// This command can
// * Extract audio to different format and quality.
// * Can copy all downloaded files to a destination folder. It will create the destination
//   folder if it does not exist.
// * Beautify the output file name?
// * Download links using multiple threads.
// * 

#include <algorithm>
#include <iostream>
#include <vector>

#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"

#include "fmt/format.h"

namespace {
    struct InputParams {
        bool extract_audio;
        unsigend int audio_quality;
        std::string audio_format;
        std::vector<std::string> links;
        std::string destination_folder;
        std::string logging_level;
        const std::string command = "youtube-dl";
    };

    template <typename T> void download(T &&inputs) {
        auto download_obj = [&inputs](auto afile) { fmt::MemoryWriter writer; };
        return;
    }

} // namespace

int main(int argc, char *argv[]) {
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    InputParams inputs;

    // clang-format off
    desc.add_options()
      ("help,h", "Print this help")
      ("verbose,v", "Display searched data.")
      ("extract-audio", "Only extracting audio if this option is on.")
      ("max-threads", po::value<unsigned int>(&numberOfThreads)->default_value(1), "Specify the maximum number of used threads.")
      ("destination-folder,d", po::value<std::string>(&inputs.Folders), "Destination folder.")
      ("logging-level,s", po::value<std::string>(&inputs.logging_level), "Specify the logging level. The default value is info.")
      ("audio-format,s", po::value<std::string>(&inputs.logging_level), "Specify the logging level. The default value is info.")
      ("audio-format,s", po::value<std::string>(&inputs.logging_level), "Specify the logging level. The default value is info.")
      ("links,l", po::value<std::vector<std::string>>(&args.links), "File extensions.")
      ("pattern,p", po::value<std::string>(&args.Pattern), "Search string pattern.")
      ("database,d", po::value<std::string>(&args.Database)->default_value(sbutils::Resources::Database), "File database.");
    // clang-format on

    po::positional_options_description p;
    p.add("pattern", -1);
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << "Usage: mlocate [options]\n";
        std::cout << desc;
        std::cout << "Examples:\n";
        std::cout << "\t mlocate -d .database -s AutoFix\n";
        std::cout << "\t mlocate -s AutoFix # if the current folder contains a file "
                     "information database i.e \".database\" folder\n";
        return 0;
    }

    return EXIT_SUCCESS;
}
