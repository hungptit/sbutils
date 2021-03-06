#pragma once

#include "Poco/PipeStream.h"
#include "Poco/Process.h"
#include "Poco/StreamCopier.h"
#include <string>
#include <vector>

namespace sbutils {
    void run(const std::string &command, const std::vector<std::string> args,
             std::string &output) {
        Poco::Pipe outPipe;
        Poco::ProcessHandle ph = Poco::Process::launch(command, args, 0, &outPipe, 0);
        Poco::PipeInputStream istr(outPipe);
        Poco::StreamCopier::copyToString(istr, output);
    }

    std::string run(const std::string &command, const std::vector<std::string> args) {
        std::string output;
        run(command, args, output);
        return output;
    }

    // (command, arguments, initial directory)
    using CommandInfo = std::tuple<std::string, std::vector<std::string>>;

    using CommandOutput = std::tuple<std::string, std::string, int>;

    CommandOutput run(CommandInfo &info, const std::string &initialDirectory) {
        Poco::Pipe outPipe, errPipe;
        Poco::ProcessHandle ph = Poco::Process::launch(std::get<0>(info), std::get<1>(info),
                                                       initialDirectory, 0, &outPipe, &errPipe);
        Poco::PipeInputStream ostr(outPipe);
        Poco::PipeInputStream estr(errPipe);
        auto errCode = Poco::Process::wait(ph);

        std::string outStr, errStr;
        Poco::StreamCopier::copyToString(ostr, outStr);
        Poco::StreamCopier::copyToString(estr, errStr);
        return std::make_tuple(outStr, errStr, errCode);
    }
}
