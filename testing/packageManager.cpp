#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <tuple>
#include <unordered_map>
#include <ctime>
#include <sstream>

#define BOOST_THREAD_VERSION 4
#include "boost/config.hpp"
#include "boost/thread.hpp"
#include "boost/thread/future.hpp"

#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"

#include "fmt/format.h"

#include "utils/Utils.hpp"
#include "utils/LevelDBIO.hpp"
#include "utils/Process.hpp"

struct BuildInfo {
  public:
    typedef boost::filesystem::path Path;
    BuildInfo(const Path &root)
        : Root(root), SrcDir(Root / Path("build")), BuildDir("/tmp/build"),
          CMake(Root / "cmake/bin/cmake"), Git(Root / ("git/bin/git")),
          CXX(Root / Path("llvm/bin/clang++")),
          CC(Root / Path("llvm/bin/clang")) {}

    void print() {
        // fmt::print("---- Build information ----\n");
        // fmt::print("Root folder: {}\n", Root);
        // fmt::print("Source folder: {}\n", SrcDir);
        // fmt::print("Build folder: {}\n", BuildDir);
        // fmt::print("CMake command: {}\n", CMake);
        // fmt::print("Git command: {}\n", Git);
        // fmt::print("CC: {}\n", CC);
        // fmt::print("CC: {}\n", CXX);
    }

    // Member data
    boost::filesystem::path Root;
    boost::filesystem::path SrcDir;
    boost::filesystem::path BuildDir;
    boost::filesystem::path CMake;
    boost::filesystem::path Git;
    boost::filesystem::path CXX;
    boost::filesystem::path CC;
    std::string BuildOptions;
};

class Package {
  public:
    typedef boost::filesystem::path Path;
    typedef std::vector<utils::CommandInfo> value_type;

    Package(const std::string &tag, const value_type &cloneCmd,
            const value_type &updateCmd, const value_type &prepareCmd,
            const value_type &buildCmd,
            const std::vector<std::string> &requiredPkgs)
        : Tag(tag), CloneCommand(cloneCmd), UpdateCommand(updateCmd),
          PrepareCommand(prepareCmd), BuildCommand(buildCmd),
          RequiredPackages(requiredPkgs) {}

    bool build(BuildInfo &info) {
        auto srcDir = info.SrcDir / Path(Tag);
        auto buildDir = info.BuildDir / Path(Tag);
        auto prefixDir = info.Root / Path(Tag);

        // Clone the source code if neccesary
        if (!boost::filesystem::is_directory(srcDir)) {
            for (auto & cmd : CloneCommand) {
                auto results = utils::run(cmd, srcDir.string());
            }
        }
                
        // Update source code
        for (auto & cmd : UpdateCommand) {
            auto results = utils::run(cmd, srcDir.string());
        }
        
        // Prepare
        for (auto & cmd : PrepareCommand) {
            auto results = utils::run(cmd, srcDir.string());
        }

        // Build
        for (auto & cmd : BuildCommand) {
            auto results = utils::run(cmd, buildDir.string());
        }
        
        return true;
    }

    void print() {
        fmt::print("Tag: {}\n", Tag);
        fmt::print("CloneCommand: \n");
        // utils::print(CloneCommand);
        // utils::print(UpdateCommand);
        // utils::print(PrepareCommand);
        // utils::print(BuildCommand);
        // fmt::print("UpdateCommand: {}\n", UpdateCommand);
        // fmt::print("PrepareCommand: {}\n", PrepareCommand);
        // fmt::print("BuildCommand: {}\n", BuildCommand);
        // fmt::print("RequiredPackages: ");
        // std::for_each(RequiredPackages.begin(), RequiredPackages.end(),
        //               [](auto item) { fmt::print("{} ", item); });
    }

  private:
    std::string Tag;
    std::vector<utils::CommandInfo> CloneCommand;
    std::vector<utils::CommandInfo> UpdateCommand;
    std::vector<utils::CommandInfo> PrepareCommand;
    std::vector<utils::CommandInfo> BuildCommand;
    std::vector<std::string> RequiredPackages;
};

class BuildManager {
  private:
    BuildInfo Info;
    std::vector<Package> Packages;
};

int main() {
    BuildInfo binfo = {"/local/projects/3p"};
    binfo.print();
    utils::CommandInfo clone{"git", {"clone", "--recursive", "https://github.com/boostorg/boost.git"}};
    utils::CommandInfo update{"git", {"pull"}};
    Package::value_type cloneCmd{update};
    // Package boost = {
    //     "boost",
    //     "git clone --recursive https://github.com/boostorg/boost.git",
    //     "git fetch; git pull",
    //     "./bootstrap.sh",
    //     ./b2 clean; ./b2 headers; ./b2 --ignore-site-config variant=release
    //     "
    //     "threading=multi toolset=clang",
    //     "make install",
    //     {}};
    // boost.print();

    // std::vector<std::string> args;
    // utils::CommandInfo info = std::make_tuple(std::string("ls"), args,
    // std::string("/local/"));
    // utils::CommandInfo info{"ls", {}, "/local/"};
    // auto results = utils::run(update);
    // std::cout << "stdout:" << std::get<0>(results) << std::endl;
    // if (std::get<2>(results)) {
    //     std::cout << "Error code:" << std::get<2>(results) << std::endl;
    //     std::cout << "stderr:" << std::get<1>(results) << std::endl;
    // }        
}
