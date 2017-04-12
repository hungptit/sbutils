#pragma once

#include <iomanip>
#include <iostream>
#include <string>
#include <tuple>
#include <utility>
#include <type_traits>

#include "DataStructures.hpp"
#include "Timer.hpp"
#include "boost/algorithm/searching/knuth_morris_pratt.hpp"

namespace {
    template <typename T, typename... Args>
    bool isValid(const sbutils::FileInfo &info, T &&first) {
        return first.isValid(info);
    }
	
    template <typename T, typename... Args>
    bool isValid(const sbutils::FileInfo &info, T &&first, Args&&... args) {
        return first.isValid(info) && isValid(info, std::forward<Args>(args)...);
    }    
}

namespace sbutils {
    template <typename T> std::string to_fixed_string(const int len, T val) {
        std::ostringstream ss;
        ss << std::setw(len) << std::setfill('0') << val;
        return ss.str();
    }

    // class KnuthMorrisPrattFilter {
    //   public:
    //     using iter_type = std::string::const_iterator;
    //     explicit KnuthMorrisPrattFilter(const std::string &pattern)
    //         : SearchAlg(pattern.begin(), pattern.end()) {}

    //     bool isValid(const FileInfo &info) { return SearchAlg(info.Path) != info.Path.end(); }

    //   private:
    //     boost::algorithm::knuth_morris_pratt<iter_type> SearchAlg;
    // };
   
    template <typename Container> class ExtFilter {
      public:
        explicit ExtFilter(Container &exts) : Extensions(exts) {}

        bool isValid(const FileInfo &info) const {
            if (Extensions.empty()) {
                return true;
            }
            return (std::find(Extensions.begin(), Extensions.end(), info.Extension) !=
                    Extensions.end());
        }

      private:
        Container Extensions;
    };

    template <typename Container> class StemFilter {
      public:
        explicit StemFilter(Container &stems) : Stems(stems) {}

        bool isValid(const FileInfo &info) const {
            if (Stems.empty()) {
                return true;
            }
            return (std::find(Stems.begin(), Stems.end(), info.Stem) != Stems.end());
        }

      private:
        std::vector<std::string> Stems;
    };

    class SimpleFilter {
      public:
        using iter_type = std::string::const_iterator;
        explicit SimpleFilter(const std::string &pattern) : Pattern(pattern) {}

        bool isValid(const FileInfo &info) const {
            return info.Path.find(Pattern) != std::string::npos;
        }

      private:
        std::string Pattern;
    };

    // Do a simple copy if there is not any constraint.
    template <typename Iterator>
    std::vector<sbutils::FileInfo> filter(Iterator begin, Iterator end) {
        return std::vector<sbutils::FileInfo>(begin, end);
    }

    template <typename Container, typename FirstConstraint, typename... Constraints>
    auto filter(Container &&data, FirstConstraint &&f1,
                                        Constraints&&... fs) {
        // sbutils::ElapsedTime<utils::MILLISECOND> t1("Filtering files: ");
		using container_type = typename std::decay<Container>::type;
		using output_type = typename container_type::value_type;
        std::vector<output_type> results;
        auto filterObj = [&f1, &fs..., &results](const auto &item) {
            if (isValid(item, f1, std::forward<Constraints>(fs)...)) {
                results.emplace_back(item);
            }
        };

        std::for_each(data.begin(), data.end(), filterObj);
        return results;
    }

    void createParentFolders(const boost::filesystem::path &dstDir,
                             const std::vector<sbutils::FileInfo> &files, bool verbose = false) {
        auto createParentObj = [&dstDir, verbose](const sbutils::FileInfo &info) {
            using namespace boost::filesystem;
            path aFile(dstDir / path(info.Path));
            path parentFolder(aFile.parent_path());
            if (!exists(parentFolder)) {
                create_directories(parentFolder);
                if (verbose) {
                    fmt::print("Create folder: {}\n", parentFolder.string());
                }
            }
        };
        std::for_each(files.begin(), files.end(), createParentObj);
    }

    bool copyAFile(const boost::filesystem::path dstDir, const sbutils::FileInfo &info,
                   const bool verbose) {
        using namespace boost::filesystem;
        const auto options = copy_option::overwrite_if_exists;
        auto srcFile = path(info.Path);
        auto dstFile = dstDir / srcFile;
        bool needCopy = true;
        if (exists(dstFile)) {
            // TODO: We do not copy a given file if the source and destination
            // have the same sizes. How can we make this algorithm better?
            needCopy &= (file_size(srcFile) != file_size(dstFile));
            if (needCopy) {
                permissions(dstFile, add_perms | owner_write);
            }
        }

        if (needCopy) {
            copy_file(srcFile, dstFile, options);
            if (verbose) {
                fmt::print("Copy {0} to {1}\n", srcFile.string(), dstFile.string());
            }
        }
        return needCopy;
    }

    bool deleteAFile(const boost::filesystem::path &parent, const sbutils::FileInfo &info,
                     const bool verbose) {
        using namespace boost::filesystem;
        auto aFile = path(info.Path);
        auto dstFile = parent / aFile;
        if (exists(dstFile)) {
            permissions(dstFile, add_perms | owner_write);
            remove(dstFile);
            if (verbose) {
                fmt::print("Delete {0}\n", dstFile.string());
            }
            return true;
        }
        return false;
    }

    auto copyFiles(const std::vector<sbutils::FileInfo> &files,
                   const boost::filesystem::path &dstDir, bool verbose = false) {
        using namespace boost::filesystem;
        size_t nfiles = 0, nbytes = 0;
        boost::system::error_code errcode;
        auto copyFileObj = [&dstDir, &nfiles, &nbytes, verbose](const sbutils::FileInfo &info)
        {
            const bool isCopied = copyAFile(dstDir, info, verbose);
            nfiles += isCopied;
            nbytes += info.Size;
        };

        std::for_each(files.begin(), files.end(), copyFileObj);
        return std::make_tuple(nfiles, nbytes);
    }

    auto deleteFiles(const std::vector<sbutils::FileInfo> &files,
                     const boost::filesystem::path &parent, bool verbose = false) {
        using namespace boost::filesystem;
        size_t nfiles = 0;
        boost::system::error_code errcode;
        for (auto const &item : files) {
            nfiles += deleteAFile(parent, item, verbose);
        }
        return nfiles;
    }
}
