#ifndef __mathalgo__matrix__
#define __mathalgo__matrix__
#include <vector>
#include <algorithm>

namespace mathalgo {
  template<class T>
  class matrix {
  public:
    matrix() {}
    matrix(unsigned int nRows, unsigned int nCols):
      m_nRows(nRows),
      m_nCols(nCols),
      m_oData(nRows*nCols, 0) {
      if (!nRows || !nCols) {
        throw std::range_error("invalid matrix size");
      }
    }

    static void identity(unsigned int nSize, matrix& result) {
      result = matrix(nSize, nSize);

      int nCount = 0;
      std::generate(result.m_oData.begin(), result.m_oData.end(),
        [&nCount, nSize]() { return !(nCount++ % (nSize + 1)); });
    }

    inline T& operator()(unsigned int nRow, unsigned int nCol) {
      if (nRow >= m_nRows || nCol >= m_nCols) {
        std::string msg = "position out of range, nRow: " + std::to_string(nRow) + " & nCol: " + std::to_string(nCol);
        throw std::out_of_range(msg.c_str());
      }

      return m_oData[nCol + m_nCols * nRow];
    }

    inline void multiply(matrix& other, matrix& result) {
      if (m_nCols != other.m_nRows) {
        throw std::domain_error("matrix dimensions are not multiplicable");
      }

      result = matrix(m_nRows, other.m_nCols);
      for (unsigned int r = 0; r < m_nRows; ++r) {
        for (unsigned int ocol = 0; ocol < other.m_nCols; ++ocol) {
          for (unsigned int c = 0; c < m_nCols; ++c) {
            result(r, ocol) += (*this)(r, c) * other(c, ocol);
          }
        }
      }
    }

    inline matrix transpose(matrix& result) {
      result = matrix(m_nCols, m_nRows);
      for (unsigned int r = 0; r < m_nRows; ++r) {
        for (unsigned int c = 0; c < m_nCols; ++c) {
          result(c, r) += (*this)(r, c);
        }
      }
    }

    inline unsigned int rows() {
      return m_nRows;
    }

    inline unsigned int cols() {
      return m_nCols;
    }

    inline std::vector<T> data() {
      return m_oData;
    }

    void print() {
      for (unsigned int r = 0; r < m_nRows; r++) {
        for (unsigned int c = 0; c < m_nCols; c++) {
          std::cout << (*this)(r, c) << "\t";
        }
        std::cout << std::endl;
      }
    }

  private:
    std::vector<T> m_oData;
    unsigned int m_nRows;
    unsigned int m_nCols;
  };
};
#endif 