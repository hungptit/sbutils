#ifndef Process_hpp
#define Process_hpp

#include "Poco/Process.h"
#include "Poco/PipeStream.h"
#include "Poco/StreamCopier.h"
#include <string>
#include <vector>

namespace Tools {
    void run(const std::string &command, const std::vector<std::string> args,
             std::string &output) {
        Poco::Pipe outPipe;
        Poco::ProcessHandle ph =
            Poco::Process::launch(command, args, 0, &outPipe, 0);
        Poco::PipeInputStream istr(outPipe);
        Poco::StreamCopier::copyToString(istr, output);
    }

    std::string run(const std::string &command,
                    const std::vector<std::string> args) {
        std::string output;
        run(command, args, output);
        return output;
    }

    // void run(const std::string &command, const std::vector<std::string> args,
    //          std::ofstream &outputFile) {
    //     Poco::Pipe outPipe;
    //     Poco::ProcessHandle ph =
    //         Poco::Process::launch(command, args, 0, &outPipe, 0);
    //     Poco::PipeInputStream istr(outPipe);
    //     Poco::StreamCopier::copyStream(istr, outputFile);
    // }
}
#endif
