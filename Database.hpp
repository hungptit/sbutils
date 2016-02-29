#ifndef Database_hpp
#define Database_hpp

#include "Utils.hpp"
#include <string>
#include <tuple>
#include <vector>

namespace {
    template <typename Reader, typename Container>
    Container read(Reader &reader, const std::string &aKey) {
        Container dict;
        auto const results = reader.read(aKey);
        if (!results.empty()) {
            std::istringstream is(results);
            utils::load<utils::IArchive, Container>(dict, is);
        }
        // std::cout << "Dictionary sizes: " << dict.size() << "\n";
        return dict;
    }
}

namespace utils {
    const std::string VertexKey = "_vertexes_";
    const std::string EdgeKey = "_edges_";
    const std::string RootKey = "_root_";

    template <typename Reader, typename Graph> class Database {
      public:
        using namespace boost::filesystem fs; 
        Database(const std::string &dataFile) : DB(dataFile)) {
            Vertexes = read(Reader, "_vertexes");
            Edges = read(Reader, "_edges");
        }

        void create(fs::path &database, const int tree_height) {
            // Search at root level and save all found file information to _root_ key.

            // Dive into a given folder and create folder hierarchy information
            // and save them to _folder_path_ key. We use task based parallelism
            // strategy to speed up this algorithm.
            
        }

            private : const std::string VertexKey = "_vertexes";
        const std::string EdgeKey = "_edges";
        Reader DB; // Should be opened in read only mode.
        std::vector<std::string> Vertexes;
        std::vector<std::tuple<int, int>> Edges;
        Graph FolderTree;
    };

    class DatabaseReader {
      public:
      private:
    };

    class DatabaseCreator {
      public:
        typedef std::string vertex_type;
        typedef std::tuple<size_t, size_t> edge_type;
        
        void create()

<<<<<<< HEAD
  private:
    Reader DB;
    std::vector<std::string> Vertexes;
    std::vector<std::tuple<int, int>> Edges;    
  }
=======
      private:
        std::vector<vertex_type> Vertexes;
        std::vector<edge_type> Edges;
        std::unordered_set<vertex_type, size_t> VertexesLookupTable;        
    };
>>>>>>> 75183dac3570ff69bb68d07245a4dd67014e6dc6
}

#endif
