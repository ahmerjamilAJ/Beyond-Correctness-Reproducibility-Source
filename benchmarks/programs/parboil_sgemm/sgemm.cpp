/* Beyond Correctness source packaging notice, 2026-10-25.
 * This is the research-adapted/amalgamated benchmark version.
 * Its upstream copyright and license notices are retained below.
 * This release adds only this notice and a line-number directive.
 * See ../../licenses/ and the benchmark catalog for notices.
 */
#line 1
#include <iostream>
#include <vector>
#include <cstddef>
/***************************************************************************
 *cr
 *cr            (C) Copyright 2010 The Board of Trustees of the
 *cr                        University of Illinois
 *cr                         All Rights Reserved
 *cr
 ***************************************************************************/

/* I/O routines for reading and writing matrices in column-major
 * layout
 */

#include<fstream>
#include<iostream>
#include<vector>

bool readColMajorMatrixFile(const char *fn, int &nr_row, int &nr_col, std::vector<float>&v)
{
  std::fstream f(fn, std::fstream::in);
  if ( !f.good() ) {
    return false;
  }

  // Read # of rows and cols
  f >> nr_row;
  f >> nr_col;

  float data;
  while (f.good() ) {
    f >> data;
    v.push_back(data);
  }
  v.pop_back(); // remove the duplicated last element
  return true;
}

bool writeColMajorMatrixFile(const char *fn, int nr_row, int nr_col, std::vector<float>&v)
{
  std::fstream f(fn, std::fstream::out);
  if ( !f.good() ) {
    return false;
  }

  // Read # of rows and cols
  f << nr_row << " "<<nr_col<<" ";

  float data;
  for (int i = 0; i < v.size(); ++i) {
    f << v[i] << ' ';
  }
  f << "\n";
  return true;

}

/***************************************************************************
 *cr
 *cr            (C) Copyright 2010 The Board of Trustees of the
 *cr                        University of Illinois
 *cr                         All Rights Reserved
 *cr
 ***************************************************************************/

/* 
 * Base C implementation of MM
 */



void basicSgemm( char transa, char transb, int m, int n, int k, float alpha, const float *A, int lda, const float *B, int ldb, float beta, float *C, int ldc )
{
  if ((transa != 'N') && (transa != 'n')) {
    std::cerr << "unsupported value of 'transa' in regtileSgemm()" << std::endl;
    return;
  }
  
  if ((transb != 'T') && (transb != 't')) {
    std::cerr << "unsupported value of 'transb' in regtileSgemm()" << std::endl;
    return;
  }
  
  for (int mm = 0; mm < m; ++mm) {
    for (int nn = 0; nn < n; ++nn) {
      float c = 0.0f;
      for (int i = 0; i < k; ++i) {
        float a = A[mm + i * lda]; 
        float b = B[nn + i * ldb];
        c += a * b;
      }
      C[mm+nn*ldc] = C[mm+nn*ldc] * beta + alpha * c;
    }
  }
}


int main(int argc, char **argv) {
  if (argc != 3) return 2;
  int ar=0, ac=0, br=0, bc=0;
  std::vector<float> A, BT;
  if (!readColMajorMatrixFile(argv[1], ar, ac, A) ||
      !readColMajorMatrixFile(argv[2], br, bc, BT)) return 2;
  if (ar < 1 || ac < 1 || br < 1 || bc != ac ||
      A.size() != (size_t)ar*ac || BT.size() != (size_t)br*bc) return 2;
  std::vector<float> C(ar*br, 0.0f);
  basicSgemm('N', 'T', ar, br, ac, 1.0f, A.data(), ar,
             BT.data(), br, 0.0f, C.data(), ar);
  return writeColMajorMatrixFile("sgemm.txt", ar, br, C) ? 0 : 3;
}
