/* Beyond Correctness source packaging notice, 2026-10-25.
 * This is the research-adapted/amalgamated benchmark version.
 * Its upstream copyright and license notices are retained below.
 * This release adds only this notice and a line-number directive.
 * See ../../licenses/ and the benchmark catalog for notices.
 */
#line 1
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define CPU_MHZ 1
/* BEEBS local library variants header

   Copyright (C) 2019 Embecosm Limited.

   Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>

   This file is part of Embench and was formerly part of the Bristol/Embecosm
   Embedded Benchmark Suite.

   SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef BEEBSC_H
#define BEEBSC_H

#include <stddef.h>

/* BEEBS fixes RAND_MAX to its lowest permitted value, 2^15-1 */

#ifdef RAND_MAX
#undef RAND_MAX
#endif
#define RAND_MAX ((1U << 15) - 1)

/* Common understanding of a "small value" (epsilon) for floating point
   comparisons. */

#define VERIFY_DOUBLE_EPS 1.0e-13
#define VERIFY_FLOAT_EPS 1.0e-5

/* Simplified assert.

   The full complexity of assert is not needed for a benchmark. See the
   discussion at:

   https://lists.librecores.org/pipermail/embench/2019-August/000007.html 

   This function just*/

#define assert_beebs(expr) { if (!(expr)) exit (1); }

#define float_eq_beebs(exp, actual) (fabsf(exp - actual) < VERIFY_FLOAT_EPS)
#define float_neq_beebs(exp, actual) !float_eq_beebs(exp, actual)
#define double_eq_beebs(exp, actual) (fabs(exp - actual) < VERIFY_DOUBLE_EPS)
#define double_neq_beebs(exp, actual) !double_eq_beebs(exp, actual)

/* Local simplified versions of library functions */

int rand_beebs (void);
void srand_beebs (unsigned int new_seed);

void init_heap_beebs (void *heap, const size_t heap_size);
int check_heap_beebs (void *heap);
void *malloc_beebs (size_t size);
void *calloc_beebs (size_t nmemb, size_t size);
void *realloc_beebs (void *ptr, size_t size);
void free_beebs (void *ptr);
#endif /* BEEBSC_H */


/*
   Local Variables:
   mode: C
   c-file-style: "gnu"
   End:
*/

/* Support header for BEEBS.

   Copyright (C) 2014 Embecosm Limited and the University of Bristol
   Copyright (C) 2019 Embecosm Limited

   Contributor James Pallister <james.pallister@bristol.ac.uk>

   Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>

   This file is part of Embench and was formerly part of the Bristol/Embecosm
   Embedded Benchmark Suite.

   SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef SUPPORT_H
#define SUPPORT_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* Include board support header if we have one */

#ifdef HAVE_BOARDSUPPORT_H
#include "boardsupport.h"
#endif

/* Benchmarks must implement verify_benchmark, which must return -1 if no
   verification is done. */

int verify_benchmark (int result);

/* Standard functions implemented for each board */

void initialise_board (void);
void start_trigger (void);
void stop_trigger (void);

/* Every benchmark implements this for one-off data initialization.  This is
   only used for initialization that is independent of how often benchmark ()
   is called. */

void initialise_benchmark (void);

/* Every benchmark implements this for cache warm up, typically calling
   benchmark several times. The argument controls how much warming up is
   done, with 0 meaning no warming. */

void warm_caches (int temperature);

/* Every benchmark implements this as its entry point. Don't allow it to be
   inlined! */

int benchmark (void) __attribute__ ((noinline));

/* Every benchmark must implement this to validate the result of the
   benchmark. */

int verify_benchmark (int res);

/* Local simplified versions of library functions */



#endif /* SUPPORT_H */

/*
   Local Variables:
   mode: C
   c-file-style: "gnu"
   End:
*/


/* BEEBS local library variants

   Copyright (C) 2019 Embecosm Limited.

   Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>

   This file is part of Embench and was formerly part of the Bristol/Embecosm
   Embedded Benchmark Suite.

   SPDX-License-Identifier: GPL-3.0-or-later */

/* These are very simple local versions of library routines, to ensure the
   code is compiled with the flags used for the benchmark.  Not all library
   routines are here, just ones that cause a lot of unecessary load, or where
   there is variation between platforms and architectures. */

#include <stddef.h>
#include <string.h>


/* Seed for the random number generator */

static long int seed = 0;

/* Heap records and sane initial values */

static void *heap_ptr = NULL;
static void *heap_end = NULL;
static size_t heap_requested = 0;


/* Yield a sequence of random numbers in the range [0, 2^15-1].

   long int is guaranteed to be at least 32 bits. The seed only ever uses 31
   bits (so is positive).

   For BEEBS this gets round different operating systems using different
   multipliers and offsets and RAND_MAX variations. */

int
rand_beebs (void)
{
  seed = (seed * 1103515245L + 12345) & ((1UL << 31) - 1);
  return (int) (seed >> 16);
}


/* Initialize the random number generator */

void
srand_beebs (unsigned int new_seed)
{
  seed = (long int) new_seed;
}


/* Initialize the BEEBS heap pointers. Note that the actual memory block is
   in the caller code. */

void
init_heap_beebs (void *heap, size_t heap_size)
{
  heap_ptr = (void *) heap;
  heap_end = (void *) ((char *) heap_ptr + heap_size);
  heap_requested = 0;
}


/* Report if malloc ever failed.

   Return non-zero (TRUE) if malloc did not reqest more than was available
   since the last call to init_heap_beebs, zero (FALSE) otherwise. */

int
check_heap_beebs (void *heap)
{
  return ((void *) ((char *) heap + heap_requested) <= heap_end);
}


/* BEEBS version of malloc.

   This is primarily to reduce library and OS dependencies. Malloc is
   generally not used in embedded code, or if it is, only in well defined
   contexts to pre-allocate a fixed amount of memory. So this simplistic
   implementation is just fine.

   Note in particular the assumption that memory will never be freed! */

void *
malloc_beebs (size_t size)
{
  void *new_ptr = heap_ptr;

  heap_requested += size;

  if (((void *) ((char *) heap_ptr + size) > heap_end) || (0 == size))
    return NULL;
  else
    {
      heap_ptr = (void *) ((char *) heap_ptr + size);
      return new_ptr;
    }
}


/* BEEBS version of calloc.

   Implement as wrapper for malloc */

void *
calloc_beebs (size_t nmemb, size_t size)
{
  void *new_ptr = malloc_beebs (nmemb * size);

  /* Calloc is defined to zero the memory. OK to use a function here, because
     it will be handled specially by the compiler anyway. */

  if (NULL != new_ptr)
    memset (new_ptr, 0, nmemb * size);

  return new_ptr;
}


/* BEEBS version of realloc.

   This is primarily to reduce library and OS dependencies. We just have to
   allocate new memory and copy stuff across. */

void *
realloc_beebs (void *ptr, size_t size)
{
  void *new_ptr = heap_ptr;

  heap_requested += size;

  if (((void *) ((char *) heap_ptr + size) > heap_end) || (0 == size))
    return NULL;
  else
    {
      heap_ptr = (void *) ((char *) heap_ptr + size);

      /* This is clunky, since we don't know the size of the original
         pointer. However it is a read only action and we know it must
         be big enough if we right off the end, or we couldn't have
         allocated here. If the size is smaller, it doesn't matter. */

      if (NULL != ptr)
	{
	  size_t i;

	  for (i = 0; i < size; i++)
	    ((char *) new_ptr)[i] = ((char *) ptr)[i];
	}

      return new_ptr;
    }
}


/* BEEBS version of free.

   For our simplified version of memory handling, free can just do nothing. */

void
free_beebs (void *ptr __attribute__ ((unused)))
{
}


/*
   Local Variables:
   mode: C
   c-file-style: "gnu"
   End:
*/

/* BEEBS aha-mont64 benchmark

   This version, copyright (C) 2013-2019 Embecosm Limited and University of
   Bristol

   Contributor James Pallister <james.pallister@bristol.ac.uk>
   Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>

   This file is part of Embench and was formerly part of the Bristol/Embecosm
   Embedded Benchmark Suite.

   SPDX-License-Identifier: GPL-3.0-or-later

   Some of this code is referenced by the book the Hacker's Delight (which
   placed it in the public domain).  See http://www.hackersdelight.org/ */



/* This scale factor will be changed to equalise the runtime of the
   benchmarks. */
#define LOCAL_SCALE_FACTOR 423

/* Computes a*b mod m using Montgomery multiplication (MM). a, b, and m
are unsigned numbers with a, b < m < 2**64, and m odd. The code does
some 128-bit arithmetic.
   The variable r is fixed at 2**64, and its log base 2 at 64.
   Works with gcc on Windows and Linux. */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

typedef uint64_t uint64;
typedef int64_t int64;

/* ---------------------------- mulul64 ----------------------------- */

/* Multiply unsigned long 64-bit routine, i.e., 64 * 64 ==> 128.
Parameters u and v are multiplied and the 128-bit product is placed in
(*whi, *wlo). If __int128 is not defined by the compiler, we fall back
to Knuth's Algorithm M from [Knu2] section 4.3.1. Derived from muldwu.c
in the Hacker's Delight collection. */

#ifdef __SIZEOF_INT128__
void
mulul64 (uint64 u, uint64 v, uint64 * whi, uint64 * wlo)
{
  unsigned __int128 result;

  result = (unsigned __int128)u * v;

  *wlo = result;
  *whi = result >> 64;
}
#else
void
mulul64 (uint64 u, uint64 v, uint64 * whi, uint64 * wlo)
{
  uint64 u0, u1, v0, v1, k, t;
  uint64 w0, w1, w2;

  u1 = u >> 32;
  u0 = u & 0xFFFFFFFF;
  v1 = v >> 32;
  v0 = v & 0xFFFFFFFF;

  t = u0 * v0;
  w0 = t & 0xFFFFFFFF;
  k = t >> 32;

  t = u1 * v0 + k;
  w1 = t & 0xFFFFFFFF;
  w2 = t >> 32;

  t = u0 * v1 + w1;
  k = t >> 32;

  *wlo = (t << 32) + w0;
  *whi = u1 * v1 + w2 + k;

  return;
}
#endif

/* ---------------------------- modul64 ----------------------------- */

uint64
modul64 (uint64 x, uint64 y, uint64 z)
{

  /* Divides (x || y) by z, for 64-bit integers x, y,
     and z, giving the remainder (modulus) as the result.
     Must have x < z (to get a 64-bit result). This is
     checked for. */

  int64 i, t;

  for (i = 1; i <= 64; i++)
    {				// Do 64 times.
      t = (int64) x >> 63;	// All 1's if x(63) = 1.
      x = (x << 1) | (y >> 63);	// Shift x || y left
      y = y << 1;		// one bit.
      if ((x | t) >= z)
	{
	  x = x - z;
	  y = y + 1;
	}
    }
  return x;			// Quotient is y.
}

/* ---------------------------- montmul ----------------------------- */

uint64
montmul (uint64 abar, uint64 bbar, uint64 m, uint64 mprime)
{

  uint64 thi, tlo, tm, tmmhi, tmmlo, uhi, ulo, ov;

  /* t = abar*bbar. */

  mulul64 (abar, bbar, &thi, &tlo);	// t = abar*bbar.

  /* Now compute u = (t + ((t*mprime) & mask)*m) >> 64.
     The mask is fixed at 2**64-1. Because it is a 64-bit
     quantity, it suffices to compute the low-order 64
     bits of t*mprime, which means we can ignore thi. */

  tm = tlo * mprime;

  mulul64 (tm, m, &tmmhi, &tmmlo);	// tmm = tm*m.

  ulo = tlo + tmmlo;		// Add t to tmm
  uhi = thi + tmmhi;		// (128-bit add).
  if (ulo < tlo)
    uhi = uhi + 1;		// Allow for a carry.

  // The above addition can overflow. Detect that here.

  ov = (uhi < thi) | ((uhi == thi) & (ulo < tlo));

  ulo = uhi;			// Shift u right
  uhi = 0;			// 64 bit positions.

// if (ov > 0 || ulo >= m)      // If u >= m,
//    ulo = ulo - m;            // subtract m from u.
  ulo = ulo - (m & -(ov | (ulo >= m)));	// Alternative
  // with no branching.

  return ulo;
}

/* ---------------------------- xbinGCD ----------------------------- */

/* C program implementing the extended binary GCD algorithm. C.f.
http://www.ucl.ac.uk/~ucahcjm/combopt/ext_gcd_python_programs.pdf. This
is a modification of that routine in that we find s and t s.t.
    gcd(a, b) = s*a - t*b,
rather than the same expression except with a + sign.
   This routine has been greatly simplified to take advantage of the
facts that in the MM use, argument a is a power of 2, and b is odd. Thus
there are no common powers of 2 to eliminate in the beginning. The
parent routine has two loops. The first drives down argument a until it
is 1, modifying u and v in the process. The second loop modifies s and
t, but because a = 1 on entry to the second loop, it can be easily seen
that the second loop doesn't alter u or v. Hence the result we want is u
and v from the end of the first loop, and we can delete the second loop.
   The intermediate and final results are always > 0, so there is no
trouble with negative quantities. Must have a either 0 or a power of 2
<= 2**63. A value of 0 for a is treated as 2**64. b can be any 64-bit
value.
   Parameter a is half what it "should" be. In other words, this function
does not find u and v st. u*a - v*b = 1, but rather u*(2a) - v*b = 1. */

void
xbinGCD (uint64 a, uint64 b, volatile uint64 * pu, volatile uint64 * pv)
{
  uint64 alpha, beta, u, v;

  u = 1;
  v = 0;
  alpha = a;
  beta = b;			// Note that alpha is
  // even and beta is odd.

  /* The invariant maintained from here on is:
     a = u*2*alpha - v*beta. */

// printf("Before, a u v = %016llx %016llx %016llx\n", a, u, v);
  while (a > 0)
    {
      a = a >> 1;
      if ((u & 1) == 0)
	{			// Delete a common
	  u = u >> 1;
	  v = v >> 1;		// factor of 2 in
	}			// u and v.
      else
	{
	  /* We want to set u = (u + beta) >> 1, but
	     that can overflow, so we use Dietz's method. */
	  u = ((u ^ beta) >> 1) + (u & beta);
	  v = (v >> 1) + alpha;
	}
//    printf("After,  a u v = %016llx %016llx %016llx\n", a, u, v);
    }

// printf("At end,    a u v = %016llx %016llx %016llx\n", a, u, v);
  *pu = u;
  *pv = v;
  return;
}

/* ------------------------------ main ------------------------------ */
static uint64 in_a, in_b, in_m;

static int benchmark_body (int  rpt);

void
warm_caches (int  heat)
{
  int  res = benchmark_body (heat);

  return;
}


int
benchmark (void)
{
  return benchmark_body (LOCAL_SCALE_FACTOR * CPU_MHZ);
}


static int __attribute__ ((noinline))
benchmark_body (int rpt)
{
  int i;
  int errors;

  for (i = 0; i < rpt; i++)
    {
      uint64 a, b, m, hr, p1hi, p1lo, p1, p, abar, bbar;
      uint64 phi, plo;
      volatile uint64 rinv, mprime;
      errors = 0;

      m = in_m;			// Must be odd.
      b = in_b;			// Must be smaller than m.
      a = in_a;			// Must be smaller than m.

      /* The simple calculation: This computes (a*b)**4 (mod m) correctly for all a,
         b, m < 2**64. */

      mulul64 (a, b, &p1hi, &p1lo);	// Compute a*b (mod m).
      p1 = modul64 (p1hi, p1lo, m);
      mulul64 (p1, p1, &p1hi, &p1lo);	// Compute (a*b)**2 (mod m).
      p1 = modul64 (p1hi, p1lo, m);
      mulul64 (p1, p1, &p1hi, &p1lo);	// Compute (a*b)**4 (mod m).
      p1 = modul64 (p1hi, p1lo, m);

      /* The MM method uses a quantity r that is the smallest power of 2
         that is larger than m, and hence also larger than a and b. Here we
         deal with a variable hr that is just half of r. This is because r can
         be as large as 2**64, which doesn't fit in one 64-bit word. So we
         deal with hr, where 2**63 <= hr <= 1, and make the appropriate
         adjustments wherever it is used.
         We fix r at 2**64, and its log base 2 at 64. It doesn't hurt if
         they are too big, it's just that some quantities (e.g., mprime) come
         out larger than they would otherwise be. */

      hr = 0x8000000000000000LL;

      /* Now, for the MM method, first compute the quantities that are
         functions of only r and m, and hence are relatively constant. These
         quantities can be used repeatedly, without change, when raising a
         number to a large power modulo m.
         First use the extended GCD algorithm to compute two numbers rinv
         and mprime, such that

         r*rinv - m*mprime = 1

         Reading this nodulo m, clearly r*rinv = 1 (mod m), i.e., rinv is the
         multiplicative inverse of r modulo m. It is needed to convert the
         result of MM back to a normal number. The other calculated number,
         mprime, is used in the MM algorithm. */

      xbinGCD (hr, m, &rinv, &mprime);	// xbinGCD, in effect, doubles hr.

      /* Do a partial check of the results. It is partial because the
         multiplications here give only the low-order half (64 bits) of the
         products. */

      if (2 * hr * rinv - m * mprime != 1)
	{
	  errors = 1;
	}

      /* Compute abar = a*r(mod m) and bbar = b*r(mod m). That is, abar =
         (a << 64)%m, and bbar = (b << 64)%m. */

      abar = modul64 (a, 0, m);
      bbar = modul64 (b, 0, m);

      p = montmul (abar, bbar, m, mprime);	/* Compute a*b (mod m). */
      p = montmul (p, p, m, mprime);	/* Compute (a*b)**2 (mod m). */
      p = montmul (p, p, m, mprime);	/* Compute (a*b)**4 (mod m). */

      /* Convert p back to a normal number by p = (p*rinv)%m. */

      mulul64 (p, rinv, &phi, &plo);
      p = modul64 (phi, plo, m);
      if (p != p1)
	errors = 1;
    }

  return errors;
}

void
initialise_benchmark (void)
{
  in_m = 0xfae849273928f89fLL;	// Must be odd.
  in_b = 0x14736defb9330573LL;	// Must be smaller than m.
  in_a = 0x0549372187237fefLL;	// Must be smaller than m.
}

// r is the number of errors therefore if r = 0 then output a 1 for correct

int
verify_benchmark (int r)
{
  return 0 == r;
}


/*
   Local Variables:
   mode: C
   c-file-style: "gnu"
   End:
*/


int main(int argc, char **argv) {
  if (argc != 2) return 2;
  FILE *fp = fopen(argv[1], "r");
  if (!fp) return 2;
  int runs = 0;
  int parsed = fscanf(fp, "%d", &runs);
  fclose(fp);
  if (parsed != 1 || runs < 1 || runs > 4) return 2;
  initialise_benchmark();
  for (int invocation = 0; invocation < runs; ++invocation) {
    volatile int result = benchmark();
    int verified = verify_benchmark(result);
    printf("invocation=%d result=%d verified=%d\n", invocation + 1, result, verified);
    if (verified != 1) return 3;
  }
  return 0;
}
