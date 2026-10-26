/* Beyond Correctness source packaging notice, 2026-10-25.
 * This is the research-adapted PolyBench/C 4.2.1-beta version.
 * PolyBench/C portions: Copyright (c) 2010-2016, Ohio State University.
 * All rights reserved. The copyright notice, redistribution conditions,
 * and disclaimer are in ../../licenses/PolyBench_LICENSE.txt, which is
 * distributed with this source. See the repository README.md for scope.
 * This release adds only this notice and a line-number directive.
 */
#line 1
/* PolyBench/C 4.2.1-beta BiCG kernel adapted to bounded external inputs.
 * Upstream: linear-algebra/kernels/bicg/bicg.c, sha256
 * 540a0f5864bc443857a5fa5ac473c7302980f32294a33c8dc10f49ddf8612115.
 * The kernel loop order and arithmetic are unchanged. Timing and the fixed
 * initializer are replaced with strict input parsing and complete output.
 */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  if (argc != 2) return 2;
  FILE *input = fopen(argv[1], "r");
  if (!input) return 2;
  int rows, cols;
  if (fscanf(input, "%d %d", &rows, &cols) != 2 ||
      rows < 2 || rows > 32 || cols < 2 || cols > 32) {
    fclose(input);
    return 2;
  }
  double *A = calloc((size_t)rows * (size_t)cols, sizeof(*A));
  double *p = calloc((size_t)cols, sizeof(*p));
  double *r = calloc((size_t)rows, sizeof(*r));
  double *s = calloc((size_t)cols, sizeof(*s));
  double *q = calloc((size_t)rows, sizeof(*q));
  if (!A || !p || !r || !s || !q) {
    free(A); free(p); free(r); free(s); free(q); fclose(input);
    return 2;
  }
  int valid = 1;
  for (int i = 0; i < rows * cols; ++i)
    if (fscanf(input, "%lf", &A[i]) != 1 || !isfinite(A[i])) valid = 0;
  for (int j = 0; j < cols; ++j)
    if (fscanf(input, "%lf", &p[j]) != 1 || !isfinite(p[j])) valid = 0;
  for (int i = 0; i < rows; ++i)
    if (fscanf(input, "%lf", &r[i]) != 1 || !isfinite(r[i])) valid = 0;
  char trailing[2];
  if (fscanf(input, "%1s", trailing) == 1) valid = 0;
  fclose(input);
  if (!valid) {
    free(A); free(p); free(r); free(s); free(q);
    return 2;
  }

  /* PolyBench/C kernel_bicg, with N=rows and M=cols. */
  for (int j = 0; j < cols; ++j) s[j] = 0;
  for (int i = 0; i < rows; ++i) {
    q[i] = 0.0;
    for (int j = 0; j < cols; ++j) {
      s[j] = s[j] + r[i] * A[(size_t)i * (size_t)cols + (size_t)j];
      q[i] = q[i] + A[(size_t)i * (size_t)cols + (size_t)j] * p[j];
    }
  }

  printf("%d %d\n", rows, cols);
  for (int i = 0; i < rows; ++i) printf("q %.17g\n", q[i]);
  for (int j = 0; j < cols; ++j) printf("s %.17g\n", s[j]);
  free(A); free(p); free(r); free(s); free(q);
  return ferror(stdout) ? 2 : 0;
}
