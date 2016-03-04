#ifndef SparseGraph_hpp_
#define SparseGraph_hpp_

#include <vector>
#include <tuple>

namespace utils {
  template <typename Index>
  class SparseGraph {
  public:
    SparseGraph() {}
    void build(std::vector<std::tuple<Index, Index>> &data, const std::size_t COL) {
      Columns.assign(COL + 1, 0);
      Rows.assign(data.size(), 0);
      Index currentRow = 0;
      for (auto const &val : data) {
        Columns[std::get<0>(val) + 1] ++;
        Rows[++currentRow] = std::get<1>(val);        
      }
      for (auto currentCol = 0; currentCol < COL; ++currentCol) {
        Columns[currentCol + 1] += Columns[currentCol];
      }
    }

    void print() const {
      for (auto currentCol = 0; currentCol < COL; ++currentCol) {
        auto begin = Columns[currentCol];
        auto end = Columns[currentCol + 1];
        std::cout << "Column[" << currentCol << "]: {";
        for (auto idx = begin; idx < end; ++idx) {
          
        }
        std::cout << "}\n";
      }
    }
  private:
    std::vector<Index> Columns;
    std::vector<Index> Rows;
  };
}
#endif
