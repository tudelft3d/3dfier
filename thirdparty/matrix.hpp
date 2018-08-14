#pragma once
#include <vector>
#include <tuple>
#include <stdexcept>
#include <algorithm>
#include <iostream>

namespace mathalgo
{
	template<class T>
	class matrix
	{
	public:
		matrix(unsigned int nRows, unsigned int nCols) : 
			m_nRows( nRows ), 
			m_nCols( nCols ), 
			m_oData( nRows*nCols, 0 )
		{
			if ( !nRows || !nCols )
			{
				throw range_error( "invalid matrix size" );
			}
		}

		static matrix identity( unsigned int nSize )
		{
			matrix oResult( nSize, nSize );

			int nCount = 0;
			std::generate( oResult.m_oData.begin(), oResult.m_oData.end(), 
				[&nCount, nSize]() { return !(nCount++%(nSize + 1)); } );

			return oResult;
		}

    inline matrix minor(int i) {
      matrix oResult(m_nRows, m_nCols);
      int n = m_nRows;
      int h = 0, k = 0;
      for (int l = 1; l < n; l++) {
        for (int j = 0; j < n; j++) {
          if (j == i)
            continue;
          oResult(h,k) = (*this)(l,j);
          k++;
          if (k == (n - 1)) {
            h++;
            k = 0;
          }
        }
      }
      return oResult;
    }

    inline T determinant(int n) {
      if (m_nCols != m_nRows) {
        throw domain_error("matrix dimensions are not the same");
      }
      matrix b(m_nRows, m_nCols);
      T sum = 0;
      if (n == 1)
        return (*this)(0,0);
      else if (n == 2)
        return ((*this)(0, 0) * (*this)(1, 1) - (*this)(0, 1) * (*this)(1, 0));
      else
        for (int i = 0; i<n; i++) {
          b = minor(i);
          sum = (T)(sum + (*this)(0, i) * pow(-1, i)*b.determinant(n - 1));	// sum = determinte matrix
        }
      return sum;
    }

    inline matrix transpose(T det) {
      matrix oResult(m_nRows, m_nCols);
      int n = m_nRows;
      for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
          oResult(i, j) = (*this)(j, i) / det;	// array d[][] = inverse matrix

      return oResult;
    }

    inline matrix cofactor(T det) {
      int n = m_nRows;
      matrix b(m_nRows, m_nCols);
      matrix c(m_nRows, m_nCols);

      for (int h = 0; h < n; h++) {
        for (int l = 0; l < n; l++) {
          int m = 0;
          int k = 0;
          for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
              if (i != h && j != l) {
                b(m, k) = (*this)(i, j);
                if (k < (n - 2))
                  k++;
                else {
                  k = 0;
                  m++;
                }
              }
          c(h, l) = (T)pow(-1, (h + l))*b.determinant(n - 1);	// c = cofactor Matrix
        }
      }
      return c.transpose(det);
    }

    inline matrix inverse() {
      matrix oResult(m_nRows, m_nCols);
      int n = m_nRows;

      T det = determinant(n);
      if (det == 0) {
        throw domain_error("inverse of Matrix is not possible");
      }
      else if (n == 1) {
        oResult(0, 0) = 1;
      }
      else {
        oResult = cofactor(det);
      }
      return oResult;
    }

		inline T& operator()(unsigned int nRow, unsigned int nCol)
		{
			if ( nRow >= m_nRows || nCol >= m_nCols )
			{
        std::string msg = "position out of range, nRow: " + std::to_string(nRow) + " & nCol: " + std::to_string(nCol);
				throw out_of_range( msg.c_str() );
			}

			return m_oData[nCol+m_nCols*nRow];
		}

		inline matrix operator*(matrix& other)
		{
			if ( m_nCols != other.m_nRows )
			{
				throw domain_error( "matrix dimensions are not multiplicable" );
			}

			matrix oResult( m_nRows, other.m_nCols );
			for ( unsigned int r = 0; r < m_nRows; ++r )
			{
				for ( unsigned int ocol = 0; ocol < other.m_nCols; ++ocol )
				{
					for ( unsigned int c = 0; c < m_nCols; ++c )
					{
						oResult(r,ocol) += (*this)(r,c) * other(c,ocol);
					}
				}
			}

			return oResult;
		}

		inline matrix transpose()
		{
			matrix oResult( m_nCols, m_nRows );
			for ( unsigned int r = 0; r < m_nRows; ++r )
			{
				for ( unsigned int c = 0; c < m_nCols; ++c )
				{
					oResult(c,r) += (*this)(r,c);
				}
			}
			return oResult;
		}

    inline matrix row(unsigned int nRow) {
      if (nRow >= m_nRows) {
        std::string msg = "position out of range, nRow: " + std::to_string(nRow);
        throw out_of_range(msg.c_str());
      }

      matrix oResult(1, m_nCols);
      for (unsigned int c = 0; c < m_nCols; ++c) {
        oResult(0, c) = (*this)(nRow, c);
      }

      return oResult;
    }

		inline unsigned int rows() 
		{
			return m_nRows;
		}

		inline unsigned int cols() 
		{
			return m_nCols;
		}

		inline std::vector<T> data()
		{
			return m_oData;
		}

		void print()
		{
			for ( unsigned int r = 0; r < m_nRows; r++ )
			{
				for ( unsigned int c = 0; c < m_nCols; c++ )
				{
					std::cout << (*this)(r,c) << "\t";
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