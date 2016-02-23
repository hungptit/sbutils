#ifndef Database_hpp
#define Database_hpp

namespace utils {

  template <typename Reader>
  class Database {
  public:
    Database(const std::string &dataFile) : DB(dataFile) {
      
    }

  private:
    Reader DB;
    std::vector<std::string> Vertexes;
    std::vector<std::tuple<int, int>> Edges;
    
  }
}

#endif
