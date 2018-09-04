#ifndef __mathalgo__Givens__
#define __mathalgo__Givens__
#include "matrix.hpp"
#include <math.h>
#include <algorithm>

namespace mathalgo
{
	using namespace std;
	template<typename T>
	class Givens
	{
	public:
		Givens() : m_oJ(2,2), m_oQ(1,1), m_oR(1,1)
		{
		}

		//Calculate the inverse of a matrix using the QR decomposition.
    void Inverse(matrix<T>& oMatrix, matrix<T>& result) {
      if (oMatrix.cols() != oMatrix.rows()) {
        throw domain_error("matrix has to be square");
      }
      matrix<T> oIdentity;
      matrix<T>::identity(oMatrix.rows(), oIdentity);
      Decompose(oMatrix);
      Solve(oIdentity, result);
    }

		// Performs QR factorization using Givens rotations.
    void Decompose(matrix<T>& oMatrix) {
      int nRows = oMatrix.rows();
      int nCols = oMatrix.cols();

      if (nRows == nCols) {
        nCols--;
      }
      else if (nRows < nCols) {
        nCols = nRows - 1;
      }

      matrix<T>::identity(nRows, m_oQ);
      m_oR = oMatrix;

      for (int j = 0; j < nCols; j++) {
        for (int i = j + 1; i < nRows; i++) {
          GivensRotation(m_oR(j, j), m_oR(i, j));
          PreMultiplyGivens(m_oR, j, i);
          PreMultiplyGivens(m_oQ, j, i);
        }
      }
      matrix<T>m_oQt;
      m_oQ.transpose(m_oQt);
      m_oQ = m_oQt;
    }
		
		// Find the solution for a matrix.
		// http://en.wikipedia.org/wiki/QR_decomposition#Using_for_solution_to_linear_inverse_problems

    void Solve(matrix<T>& oMatrix, matrix<T>& result) {
      matrix<T> oQt, oQtM;
      m_oQ.transpose(oQt);
      oQt.multiply(oMatrix, oQtM);
      int nCols = m_oR.cols();
      result = matrix<T>(1, nCols);
      for (int i = nCols - 1; i >= 0; i--) {
        result(0, i) = oQtM(i, 0);
        for (int j = i + 1; j < nCols; j++) {
          result(0, i) -= result(0, j) * m_oR(i, j);
        }
        result(0, i) /= m_oR(i, i);
      }
    }

    const matrix<T>& GetQ() {
      return m_oQ;
    }

    const matrix<T>& GetR() {
      return m_oR;
    }

	private:
		// Givens rotation is a rotation in the plane spanned by two coordinates axes.
		// http://en.wikipedia.org/wiki/Givens_rotation
    void GivensRotation(T a, T b) {
      T t, s, c;
      if (b == 0) {
        c = (a >= 0) ? 1 : -1;
        s = 0;
      }
      else if (a == 0) {
        c = 0;
        s = (b >= 0) ? -1 : 1;
      }
      else if (abs(b) > abs(a)) {
        t = a / b;
        s = -1 / sqrt(1 + t * t);
        c = -s * t;
      }
      else {
        t = b / a;
        c = 1 / sqrt(1 + t * t);
        s = -c * t;
      }
      m_oJ(0, 0) = c; m_oJ(0, 1) = -s;
      m_oJ(1, 0) = s; m_oJ(1, 1) = c;
    }

		// Get the premultiplication of a given matrix by the Givens rotation.
    void PreMultiplyGivens(matrix<T>& oMatrix, int i, int j) {
      int nRowSize = oMatrix.cols();

      for (int nRow = 0; nRow < nRowSize; nRow++) {
        double nTemp = oMatrix(i, nRow) * m_oJ(0, 0) + oMatrix(j, nRow) * m_oJ(0, 1);
        oMatrix(j, nRow) = oMatrix(i, nRow) * m_oJ(1, 0) + oMatrix(j, nRow) * m_oJ(1, 1);
        oMatrix(i, nRow) = nTemp;
      }
    }

	private:
		matrix<T> m_oQ, m_oR, m_oJ;
	};
}
#endif