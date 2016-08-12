
#ifndef Utils_DataStructures_hpp_
#define Utils_DataStructures_hpp_

#include <string>
#include <utility>
#include <vector>

#include "cereal/archives/binary.hpp"
#include "cereal/archives/json.hpp"
#include "cereal/archives/portable_binary.hpp"
#include "cereal/archives/xml.hpp"
#include "cereal/types/array.hpp"
#include "cereal/types/chrono.hpp"
#include "cereal/types/deque.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/vector.hpp"

#include "boost/filesystem.hpp"

#include "graph/SparseGraph.hpp"

namespace utils {

    using DefaultIArchive = cereal::BinaryInputArchive;
    using DefaultOArchive = cereal::BinaryOutputArchive;

    /**
     * Defininition for FileInfo data structure.
     *
     */

    struct FileInfo {
        using String = std::string;
        
        FileInfo()
            : Permissions(), Size(), Path(), Stem(), Extension(), TimeStamp() {}

        template <typename T1, typename T2, typename T3, typename T4,
                  typename T5, typename T6>
        FileInfo(T1 &&perms, T2 &&sizes, T3 &&path, T4 &&stem, T5 &&ext,
                    T6 &&timeStamp) noexcept
            : Permissions(std::forward<T1>(perms)),
              Size(std::forward<T2>(sizes)), Path(std::forward<T3>(path)),
              Stem(std::forward<T4>(stem)), Extension(std::forward<T5>(ext)),
              TimeStamp(std::forward<T6>(timeStamp)) {}

        FileInfo(const FileInfo &info) noexcept
            : Permissions(info.Permissions), Size(info.Size), Path(info.Path),
              Stem(info.Stem), Extension(info.Extension),
              TimeStamp(info.TimeStamp) {}

        FileInfo(FileInfo &&info) noexcept
            : Permissions(info.Permissions), Size(info.Size),
              Path(std::move(info.Path)), Stem(info.Stem),
              Extension(info.Extension), TimeStamp(info.TimeStamp) {}

        FileInfo &operator=(const FileInfo &rhs) {
            // if (*this == rhs) {
            //     return *this;
            // }
            this->Permissions = rhs.Permissions;
            this->Size = rhs.Size;
            this->Path = rhs.Path;
            this->Stem = rhs.Stem;
            this->Extension = rhs.Extension;
            this->TimeStamp = rhs.TimeStamp;
            return *this;
        }

        template <typename Archive> void serialize(Archive &ar) {
            ar(cereal::make_nvp("perms", Permissions),
               cereal::make_nvp("size", Size), cereal::make_nvp("path", Path),
               cereal::make_nvp("stem", Stem),
               cereal::make_nvp("ext", Extension),
               cereal::make_nvp("time_stamp", TimeStamp));
        }

        // Data members
        int Permissions;
        uintmax_t Size;
        String Path;
        String Stem;
        String Extension;
        std::time_t TimeStamp;
    };
   
    // A file path must be unique.
    bool operator<(const FileInfo &lhs, const FileInfo &rhs) {
        return (lhs.Path < rhs.Path);
    }

    bool operator==(const FileInfo &lhs,
                    const FileInfo &rhs) {
        return (lhs.Size == rhs.Size) && (lhs.Path == rhs.Path);
    }


    /**
     * Definition for the folder hirarchy.
     *
     */

    template <typename index_type> struct Vertex {
        std::string Path;
        std::vector<FileInfo> Files;

        explicit Vertex() : Path(), Files() {}
        Vertex(const Vertex &data) : Path(data.Path), Files(data.Files) {}

        template <typename T>
        Vertex(T &&data)
            : Path(std::move(data.Path)), Files(std::move(data.Files)) {}

        template <typename T1, typename T2>
        Vertex(T1 &&aPath, T2 &&files)
            : Path(std::move(aPath)), Files(std::move(files)) {}

        template <typename Archive> void serialize(Archive &ar) {
            ar(cereal::make_nvp("path", Path),
               cereal::make_nvp("files", Files));
        }
    };

    template <typename itype> struct FolderHierarchy {
        using index_type = itype;
        using vertex_type = Vertex<index_type>;
        using vertex_container = std::vector<Vertex<index_type>>;

        using edge_type = graph::BasicEdgeData<index_type>;
        using edge_container = std::vector<edge_type>;

        using file_container = std::vector<FileInfo>;

        template <typename T1, typename T2>
        FolderHierarchy(T1 &&vertexes, T2 &&edges)
            : Vertexes(std::move(vertexes)),
              Graph(std::move(edges), Vertexes.size(), true), AllFiles() {
            update();
        }

        void update() {
            std::size_t counter = 0;
            std::for_each(Vertexes.cbegin(), Vertexes.cend(),
                          [&counter](auto const &aFolder) {
                              counter += aFolder.Files.size();
                          });
            AllFiles.reserve(counter);

            // TODO: Benchmark std::copy, emplace_back, , back_inserter, and
            // insert approaches.
            std::for_each(
                Vertexes.cbegin(), Vertexes.cend(), [&](auto const &aFolder) {
                    std::for_each(
                        aFolder.Files.cbegin(), aFolder.Files.cend(),
                        [&](auto const &item) { AllFiles.emplace_back(item); });
                });

            std::sort(AllFiles.begin(), AllFiles.end(),
                      [](const auto &first, const auto &second) {
                          return first.Path < second.Path;
                      });
        }

        template <typename Archive> void serialize(Archive &ar) {
            ar(cereal::make_nvp("vertexes", Vertexes),
               cereal::make_nvp("graph", Graph),
               cereal::make_nvp("all_files", AllFiles));
        }

        void info() const {
            fmt::print("Number of vertexes: {}\n", Vertexes.size());
            fmt::print("Number of files: {}\n", AllFiles.size());
        }

        // Each vertex will have its path and files at the root level. We need
        // to traverse the tree to get all files or folders that belong to a
        // given folder.
        vertex_container Vertexes;

        // A tree that represents the folder hierarchy.
        graph::SparseGraph<index_type, edge_type> Graph;

        // All files that belong to given root folders.
        file_container AllFiles;
    };

    struct RootFolder {
        using value_type = boost::filesystem::path;
        value_type Path;
        std::vector<value_type> Files;
        std::vector<value_type> Folders;

        explicit RootFolder() : Path(), Files(), Folders() {}

        explicit RootFolder(const RootFolder &data)
            : Path(data.Path), Files(data.Files), Folders(data.Folders) {}

        template <typename T>
        RootFolder(T &&data)
            : Path(std::move(data.Path)), Files(std::move(data.Files)),
              Folders(std::move(data.Folders)) {}

        void update(const value_type &aPath) {
            Path = aPath;
            boost::filesystem::directory_iterator endIter;
            boost::filesystem::directory_iterator dirIter(Path);
            for (; dirIter != endIter; ++dirIter) {
                auto currentPath = dirIter->path();
                if (boost::filesystem::is_directory(currentPath)) {
                    Folders.emplace_back(std::move(currentPath));
                } else if (boost::filesystem::is_regular_file(currentPath)) {
                    Files.emplace_back(std::move(currentPath));
                } else {
                    // Ignore this case
                }
            }
        }
    };

    template <typename OArchive, typename T, typename Stream>
    void serialize(const T &data, const std::string &title, Stream &os) {
        utils::DefaultOArchive oar(os);
        oar(title, data);
    }

    template <typename OArchive, typename T, typename Stream>
    void serialize(const T &data, Stream &os) {
        utils::DefaultOArchive oar(os);
        oar(data);
    }

    template <typename OArchive, typename T>
    void print(const T &data, const std::string &title) {
        std::stringstream output;
        {
            OArchive oar(output);
            oar(cereal::make_nvp(title, data));
        }

        fmt::print("{}\n", output.str());
    }
}

namespace std {
    template <> struct hash<utils::FileInfo> {
        using value_type = utils::FileInfo;
        typedef std::size_t result_type;

        result_type operator()(const value_type &aKey) const {
            // We only care about the size and path of files.
            result_type const h1(std::hash<uintmax_t>()(aKey.Size));
            result_type const h2(std::hash<std::string>()(aKey.Path));
            return h1 ^ (h2 << 1);
        }
    };
}

#endif
