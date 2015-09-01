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

#include "cppformat/format.h"

#include "utils/Utils.hpp"
#include "utils/FindUtils.hpp"
#include "utils/LevelDBIO.hpp"
#include "utils/Process.hpp"
#include "Finder.hpp"

struct BuildInfo {
  public:
    typedef boost::filesystem::path Path;
    BuildInfo(const Path &root)
        : Root(root), SrcDir(Root / Path("build")), BuildDir("/tmp/build"),
          CMake(Root / "cmake/bin/cmake"), Git(Root / ("git/bin/git")),
          CXX(Root / Path("llvm/bin/clang++")),
          CC(Root / Path("llvm/bin/clang")) {}

    void print() {
        fmt::print("---- Build information ----\n");
        fmt::print("Root folder: {}\n", Root);
        fmt::print("Source folder: {}\n", SrcDir);
        fmt::print("Build folder: {}\n", BuildDir);
        fmt::print("CMake command: {}\n", CMake);
        fmt::print("Git command: {}\n", Git);
        fmt::print("CC: {}\n", CC);
        fmt::print("CC: {}\n", CXX);
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
    Package(const std::string &tag, const std::string &cloneCmd,
            const std::string &updateCmd, const std::string &prepareCmd,
            const std::string &buildCmd, const std::string &installCmd,
            const std::vector<std::string> &requiredPkgs)
        : Tag(tag), CloneCommand(cloneCmd), UpdateCommand(updateCmd),
          PrepareCommand(prepareCmd), BuildCommand(buildCmd),
          InstallCommand(installCmd), RequiredPackages(requiredPkgs) {}

    bool build(BuildInfo &info) {
        auto srcDir = info.SrcDir / Path(Tag);
        auto buildDir = info.BuildDir / Path(Tag);

        // Clone or update the source code
        if (boost::filesystem::is_directory(srcDir)) {
            // Update the source code
        } else {
            // Clone the source code
        }

        // Prepare

        // Build

        // Install

        return true;
    }

    void print() {
        fmt::print("Tag: {}\n", Tag);
        fmt::print("CloneCommand: {}\n", CloneCommand);
        fmt::print("UpdateCommand: {}\n", UpdateCommand);
        fmt::print("PrepareCommand: {}\n", PrepareCommand);
        fmt::print("BuildCommand: {}\n", BuildCommand);
        fmt::print("InstallCommand: {}\n", InstallCommand);
        fmt::print("RequiredPackages: ");
        std::for_each(RequiredPackages.begin(), RequiredPackages.end(),
                      [](auto item) { fmt::print("{} ", item); });
    }

  private:
    std::string Tag;
    std::string CloneCommand;
    std::string UpdateCommand;
    std::string PrepareCommand;
    std::string BuildCommand;
    std::string InstallCommand;
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
    Package boost = {
        "boost",
        "git clone --recursive https://github.com/boostorg/boost.git",
        "git fetch; git pull",
        "./bootstrap.sh",
        "./b2 clean; ./b2 headers; ./b2 --ignore-site-config variant=release "
        "threading=multi toolset=clang",
        "make install",
        {}};
    boost.print();
}
