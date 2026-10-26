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

/* BEEBS nbody benchmark

   This version, copyright (C) 2014-2019 Embecosm Limited and University of
   Bristol

   Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>

   This file is part of Embench and was formerly part of the Bristol/Embecosm
   Embedded Benchmark Suite.

   SPDX-License-Identifier: GPL-3.0-or-later

   The original source code for this benchmark can be found here:

     http://benchmarksgame.alioth.debian.org/

   and was released under the following licence, disclaimers, and
   copyright:

   Revised BSD license

   This is a specific instance of the Open Source Initiative (OSI) BSD
   license template http://www.opensource.org/licenses/bsd-license.php

   Copyright 2004-2009 Brent Fulgham
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the
   distribution.

   Neither the name of "The Computer Language Benchmarks Game" nor the
   name of "The Computer Language Shootout Benchmarks" nor the names
   of its contributors may be used to endorse or promote products
   derived from this software without specific prior written
   permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
   FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
   COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
   STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
   OF THE POSSIBILITY OF SUCH DAMAGE.  */

#include <math.h>
#include <stdlib.h>



#define LOCAL_SCALE_FACTOR 1

#define PI 3.141592653589793
#define SOLAR_MASS ( 4 * PI * PI )
#define DAYS_PER_YEAR 365.24

struct body
{
  double x[3], fill, v[3], mass;
};

static struct body solar_bodies[] = {
  /* sun */
  {
   .x = {0., 0., 0.},
   .v = {0., 0., 0.},
   .mass = SOLAR_MASS},
  /* jupiter */
  {
   .x = {4.84143144246472090e+00,
	 -1.16032004402742839e+00,
	 -1.03622044471123109e-01},
   .v = {1.66007664274403694e-03 * DAYS_PER_YEAR,
	 7.69901118419740425e-03 * DAYS_PER_YEAR,
	 -6.90460016972063023e-05 * DAYS_PER_YEAR},
   .mass = 9.54791938424326609e-04 * SOLAR_MASS},
  /* saturn */
  {
   .x = {8.34336671824457987e+00,
	 4.12479856412430479e+00,
	 -4.03523417114321381e-01},
   .v = {-2.76742510726862411e-03 * DAYS_PER_YEAR,
	 4.99852801234917238e-03 * DAYS_PER_YEAR,
	 2.30417297573763929e-05 * DAYS_PER_YEAR},
   .mass = 2.85885980666130812e-04 * SOLAR_MASS},
  /* uranus */
  {
   .x = {1.28943695621391310e+01,
	 -1.51111514016986312e+01,
	 -2.23307578892655734e-01},
   .v = {2.96460137564761618e-03 * DAYS_PER_YEAR,
	 2.37847173959480950e-03 * DAYS_PER_YEAR,
	 -2.96589568540237556e-05 * DAYS_PER_YEAR},
   .mass = 4.36624404335156298e-05 * SOLAR_MASS},
  /* neptune */
  {
   .x = {1.53796971148509165e+01,
	 -2.59193146099879641e+01,
	 1.79258772950371181e-01},
   .v = {2.68067772490389322e-03 * DAYS_PER_YEAR,
	 1.62824170038242295e-03 * DAYS_PER_YEAR,
	 -9.51592254519715870e-05 * DAYS_PER_YEAR},
   .mass = 5.15138902046611451e-05 * SOLAR_MASS}
};

static const int BODIES_SIZE =
  sizeof (solar_bodies) / sizeof (solar_bodies[0]);

void
offset_momentum (struct body *bodies, unsigned int nbodies)
{
  unsigned int i, k;
  for (i = 0; i < nbodies; ++i)
    for (k = 0; k < 3; ++k)
      bodies[0].v[k] -= bodies[i].v[k] * bodies[i].mass / SOLAR_MASS;
}


double
bodies_energy (struct body *bodies, unsigned int nbodies)
{
  double dx[3], distance, e = 0.0;
  unsigned int i, j, k;

  for (i = 0; i < nbodies; ++i)
    {
      e += bodies[i].mass * (bodies[i].v[0] * bodies[i].v[0]
			     + bodies[i].v[1] * bodies[i].v[1]
			     + bodies[i].v[2] * bodies[i].v[2]) / 2.;

      for (j = i + 1; j < nbodies; ++j)
	{
	  for (k = 0; k < 3; ++k)
	    dx[k] = bodies[i].x[k] - bodies[j].x[k];

	  distance = sqrt (dx[0] * dx[0] + dx[1] * dx[1] + dx[2] * dx[2]);
	  e -= (bodies[i].mass * bodies[j].mass) / distance;
	}
    }
  return e;
}

void
initialise_benchmark (void)
{
}



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
  int j;
  double tot_e = 0.0;

  for (j = 0; j < rpt; j++)
    {
      int i;
      offset_momentum (solar_bodies, BODIES_SIZE);
      /*printf("%.9f\n", bodies_energy(solar_bodies, BODIES_SIZE)); */
      tot_e = 0.0;
      for (i = 0; i < 100; ++i)
	tot_e += bodies_energy (solar_bodies, BODIES_SIZE);
      /*printf("%.9f\n", bodies_energy(solar_bodies, BODIES_SIZE)); */
    }
  /* Result is known good value for total energy. */
  return double_eq_beebs(tot_e, -16.907516382852478);
}


int
verify_benchmark (int tot_e_ok)
{
  int i, j;
  /* print expected values */
  // printf("static struct body solar_bodies[] = {\n");
  // for (i=0; i<BODIES_SIZE; i++) {
  //    printf("  {\n");
  //    printf("    .x = { %.30g, %.30g, %.30g },\n", solar_bodies[i].x[0],  solar_bodies[i].x[1],  solar_bodies[i].x[2]);
  //    printf("    .v = { %.30g, %.30g, %.30g },\n", solar_bodies[i].v[0],  solar_bodies[i].v[1],  solar_bodies[i].v[2]);
  //    printf("    .mass = %.30g\n", solar_bodies[i].mass);
  //    printf("  }");
  //    if (i<BODIES_SIZE-1) printf(",");
  //    printf("\n");
  // }
  // printf("};\n");
  static struct body expected[] = {
    {
     .x = {0, 0, 0},
     .v =
     {-0.000387663407198742665776131088862,
      -0.0032753590371765706722173572274,
      2.39357340800030020670947916717e-05},
     .mass = 39.4784176043574319692197605036},
    {
     .x =
     {4.84143144246472090230781759601, -1.16032004402742838777840006514,
      -0.103622044471123109232735259866},
     .v =
     {0.606326392995832019749968821998, 2.81198684491626016423992950877,
      -0.0252183616598876288172892401462},
     .mass = 0.0376936748703894930478952574049},
    {
     .x =
     {8.34336671824457987156620220048, 4.1247985641243047894022311084,
      -0.403523417114321381049535375496},
     .v =
     {-1.01077434617879236000703713216, 1.82566237123041186229954746523,
      0.00841576137658415351916474378413},
     .mass = 0.0112863261319687668143840753032},
    {
     .x =
     {12.8943695621391309913406075793, -15.1111514016986312469725817209,
      -0.223307578892655733682204299839},
     .v =
     {1.08279100644153536414648897335, 0.868713018169608219842814378353,
      -0.0108326374013636358983880825235},
     .mass = 0.0017237240570597111687795033319},
    {
     .x =
     {15.3796971148509165061568637611, -25.9193146099879641042207367718,
      0.179258772950371181309492385481},
     .v =
     {0.979090732243897976516677772452, 0.594698998647676169149178804219,
      -0.0347559555040781037460462243871},
     .mass = 0.00203368686992463042206846779436}
  };

  /* Check we have the correct total energy and we have set up the
     solar_bodies array correctly. */

  if (tot_e_ok)
    for (i = 0; i < BODIES_SIZE; i++)
      {
	for (j = 0; j < 3; j++)
	  {
	    if (double_neq_beebs(solar_bodies[i].x[j], expected[i].x[j]))
	      return 0;
	    if (double_neq_beebs(solar_bodies[i].v[j], expected[i].v[j]))
	      return 0;
	  }
	if (double_neq_beebs(solar_bodies[i].mass, expected[i].mass))
	  return 0;
      }
  else
    return 0;

  return 1;
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
