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

/* BEEBS matmult benchmark

   This version, copyright (C) 2013-2019 Embecosm Limited and University of
   Bristol

   Contributor James Pallister <james.pallister@bristol.ac.uk>
   Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>

   This file is part of Embench and was formerly part of the Bristol/Embecosm
   Embedded Benchmark Suite.

   SPDX-License-Identifier: GPL-3.0-or-later

   Original code from: WCET Benchmarks,
       http://www.mrtc.mdh.se/projects/wcet/benchmarks.html
   Permission to license under GPL obtained by email from Björn Lisper
 */

/* matmult.c */
/* was mm.c! */


/*----------------------------------------------------------------------*
 * To make this program compile under our assumed embedded environment,
 * we had to make several changes:
 * - Declare all functions in ANSI style, not K&R.
 *   this includes adding return types in all cases!
 * - Declare function prototypes
 * - Disable all output
 * - Disable all UNIX-style includes
 *
 * This is a program that was developed from mm.c to matmult.c by
 * Thomas Lundqvist at Chalmers.
 *----------------------------------------------------------------------*/

#include <string.h>


/* This scale factor will be changed to equalise the runtime of the
   benchmarks. */
#define LOCAL_SCALE_FACTOR 46
#define UPPERLIMIT 20
#define RANDOM_VALUE (RandomInteger ())
#define ZERO 0
#define MOD_SIZE 8095
typedef long matrix[UPPERLIMIT][UPPERLIMIT];

int
values_match (long v1, long v2)
{
  return (v1 == v2);
}


/*
 * MATRIX MULTIPLICATION BENCHMARK PROGRAM:
 * This program multiplies 2 square matrices resulting in a 3rd
 * matrix. It tests a compiler's speed in handling multidimensional
 * arrays and simple arithmetic.
 */




int Seed;
matrix ArrayA_ref, ArrayA, ArrayB_ref, ArrayB, ResultArray;

void Multiply (matrix A, matrix B, matrix Res);
void InitSeed (void);
void Test (matrix A, matrix B, matrix Res);
void Initialize (matrix Array);
int RandomInteger (void);

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

  for (i = 0; i < rpt; i++)
    {
      memcpy (ArrayA, ArrayA_ref,
	      UPPERLIMIT * UPPERLIMIT * sizeof (ArrayA[0][0]));
      memcpy (ArrayB, ArrayB_ref,
	      UPPERLIMIT * UPPERLIMIT * sizeof (ArrayA[0][0]));

      Test (ArrayA, ArrayB, ResultArray);
    }

  return 0;
}


/*
 * Initializes the seed used in the random number generator.
 */
void
InitSeed (void)
{
  Seed = 0;
}

/*
 * Runs a multiplication test on an array.  Calculates and prints the
 * time it takes to multiply the matrices.
 */
void
Test (matrix A, matrix B, matrix Res)
{
  Multiply (A, B, Res);
}

/*
 * Generates random integers between 0 and 8095
 */
int
RandomInteger (void)
{
  Seed = ((Seed * 133) + 81) % MOD_SIZE;
  return (Seed);
}

/*
 * Multiplies arrays A and B and stores the result in ResultArray.
 */
void
Multiply (matrix A, matrix B, matrix Res)
{
  register int Outer, Inner, Index;

  for (Outer = 0; Outer < UPPERLIMIT; Outer++)
    for (Inner = 0; Inner < UPPERLIMIT; Inner++)
      {
	Res[Outer][Inner] = ZERO;
	for (Index = 0; Index < UPPERLIMIT; Index++)
	  Res[Outer][Inner] += A[Outer][Index] * B[Index][Inner];
      }
}

void
initialise_benchmark ()
{
  InitSeed ();
  int OuterIndex, InnerIndex;

  for (OuterIndex = 0; OuterIndex < UPPERLIMIT; OuterIndex++)
    for (InnerIndex = 0; InnerIndex < UPPERLIMIT; InnerIndex++)
      ArrayA_ref[OuterIndex][InnerIndex] = RANDOM_VALUE;
  for (OuterIndex = 0; OuterIndex < UPPERLIMIT; OuterIndex++)
    for (InnerIndex = 0; InnerIndex < UPPERLIMIT; InnerIndex++)
      ArrayB_ref[OuterIndex][InnerIndex] = RANDOM_VALUE;
}

int
verify_benchmark (int unused)
{
  int i, j;
  matrix exp = {
    {291018000, 315000075, 279049970, 205074215, 382719905,
     302595865, 348060915, 308986330, 343160760, 307099935,
     292564810, 240954510, 232755815, 246511665, 328466830,
     263664375, 324016395, 334656070, 285978755, 345370360},
    {252241835, 333432715, 299220275, 247745815, 422508990,
     316728505, 359662270, 277775280, 323336795, 320656600,
     249903690, 251499360, 242195700, 263484280, 348207635,
     289485100, 328607555, 300799835, 269351410, 305703460},
    {304901010, 316252815, 263230275, 208939015, 421993740,
     335002930, 348571170, 280992155, 289749970, 259701175,
     295249990, 310900035, 250896625, 250154105, 315096035,
     236364800, 312879355, 312580685, 275998435, 344137885},
    {286700525, 325985600, 253054970, 224361490, 353502130,
     306544290, 323492140, 259123905, 307731610, 282414410,
     281127810, 246936935, 207890815, 233789540, 339836730,
     277296350, 319925620, 307470895, 290537580, 292297535},
    {272571255, 377663320, 304545985, 263001340, 375034885,
     325423710, 410620380, 313191730, 356989815, 308508355,
     218003850, 272487135, 266000220, 264734710, 367539620,
     304146675, 355295500, 276019740, 251415695, 301225235},
    {272547900, 321522300, 288294345, 247748015, 389912855,
     331874890, 370798315, 315467255, 367554485, 311947660,
     258809685, 270536510, 256730515, 287143040, 363087030,
     285672775, 353670120, 304219695, 274897255, 324684660},
    {233123995, 227142480, 212655155, 198592290, 345335250,
     302661845, 253374925, 233243305, 233750030, 224590040,
     200404820, 250791135, 234405760, 211723645, 280630165,
     185245875, 296423665, 276278575, 252368265, 278726535},
    {277690535, 339615440, 320921550, 307114315, 400187215,
     334374655, 376286920, 295993530, 362988020, 356272700,
     293965465, 261574710, 259690975, 263037705, 416748985,
     274683275, 385571030, 402782385, 323927010, 362778710},
    {267168970, 323401680, 279474330, 201934365, 362624300,
     330736145, 371793675, 299650280, 333646005, 264791490,
     215918320, 277512760, 264068435, 234555295, 321772515,
     217507025, 310372440, 317544750, 245525965, 343183435},
    {281293570, 326519505, 233494705, 238516065, 297038200,
     266273420, 349521550, 259343530, 306032255, 266397915,
     210274920, 263743085, 231689610, 251949545, 293562740,
     226822900, 309225440, 286212000, 206108715, 236678985},
    {288404350, 310319375, 282695670, 244150740, 426489380,
     387525790, 342018190, 326086505, 352250260, 319997735,
     300645835, 284822660, 271837440, 274000415, 361826730,
     252399600, 348582320, 375813820, 316588255, 322499110},
    {273368780, 329706295, 288668335, 234501665, 381962610,
     343186285, 337520205, 259637405, 295755465, 284778105,
     205310525, 249598310, 256662470, 251533535, 336159770,
     249342150, 333559450, 329296590, 278254845, 300673860},
    {318589575, 315522800, 260632295, 250009765, 337127730,
     312810490, 346698590, 260810030, 388289910, 337081285,
     283635410, 208148610, 234123865, 259653165, 370115255,
     243311450, 377808245, 358786770, 286839730, 321912835},
    {229541925, 253967450, 223002545, 202302515, 303446955,
     268472740, 285580065, 211013405, 287677960, 279773910,
     227377310, 197461135, 222469715, 179536615, 306957380,
     178407075, 281051570, 279718120, 234868230, 288991535},
    {290692955, 317729070, 297868235, 213450065, 469270935,
     375344910, 326987580, 334565680, 325300040, 290325655,
     254703825, 284914960, 245773820, 276641510, 323510795,
     271034400, 337424250, 360011440, 281515520, 331261535},
    {287075125, 313194850, 269889345, 208109115, 420653930,
     331900290, 355440665, 318065155, 343785360, 302163035,
     308959360, 312666110, 268997740, 288557415, 370158305,
     205012650, 318198795, 384484520, 316450105, 378714460},
    {278680580, 356815220, 307597060, 216073365, 390879235,
     358775185, 358895230, 306434180, 315569040, 272688130,
     249424325, 274584610, 273530970, 265450585, 325127920,
     312802050, 317134900, 298518590, 269975470, 332586535},
    {245629780, 267021570, 234689035, 208808065, 366356035,
     267059560, 349348005, 270158755, 348048340, 291550930,
     272717800, 259714410, 236033845, 280627610, 335089770,
     176610475, 259339950, 322752840, 236218295, 329687310},
    {226517370, 272306005, 271484080, 216145515, 400972075,
     288475645, 332969550, 338410905, 329052205, 330392265,
     306488095, 271979085, 232795960, 257593945, 339558165,
     202700275, 320622065, 386350450, 315344865, 329233410},
    {224852610, 231292540, 236945875, 243273740, 336327040,
     305144680, 248261920, 191671605, 241699245, 263085200,
     198883715, 175742885, 202517850, 172427630, 296304160,
     209188850, 326546955, 252990460, 238844535, 289753485}
  };

  return 0 == memcmp (ResultArray, exp,
		      UPPERLIMIT * UPPERLIMIT * sizeof (exp[0][0]));
}

/* vim: set ts=3 sw=3 et: */


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
