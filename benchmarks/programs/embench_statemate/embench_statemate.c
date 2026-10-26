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

/* BEEBS statemate benchmark

   This version, copyright (C) 2014-2019 Embecosm Limited and University of
   Bristol

   Contributor James Pallister <james.pallister@bristol.ac.uk>
   Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>

   This file is part of Embench and was formerly part of the Bristol/Embecosm
   Embedded Benchmark Suite.

   SPDX-License-Identifier: GPL-3.0-or-later */

/* MDH WCET BENCHMARK SUITE. */

/*
 *----------------------------------------------------------
 *
 * statemate.c
 *
 * This code was automatically generated by
 * the STAtechart Real-time-Code generator STARC
 * which was developed at C-LAB.
 *
 * The original StateChart specifies an experimental
 * car window lift control.
 *
 * Modified and maintained by
 * Friedhelm Stappert
 * C-LAB, Paderborn, Germany
 * fst@c-lab.de
 *
 * Modifications:
 * o '#define float int' so we don't need a
 *   floating point library.
 *
 * o modified the Bitlist functions.
 *   'Bitlist is now just a array of char's
 *  so we don't need the BitList library.
 *
 *----------------------------------------------------------
 */

#include <string.h>


/* This scale factor will be changed to equalise the runtime of the
   benchmarks. */
#define LOCAL_SCALE_FACTOR 1964

/*
** actually, we don't really need floating point here
*/
#define float int

static char Bitlist[64];
#define SYS_bit_get(a,b) (a)[(b)]
#define SYS_bit_clr(a,b) (a)[(b)] = 0
#define SYS_bit_set(a,b) (a)[(b)] = 1
#define SYS_bit_cpy(a1,i1,a2,i2) (a1)[(i1)] = (a2)[(i2)]


#define active_KINDERSICHERUNG_CTRL_IDX 10
#define active_KINDERSICHERUNG_CTRL_copy_IDX 11
#define active_KINDERSICHERUNG_CTRL_old_IDX 12
#define active_FH_TUERMODUL_CTRL_IDX 13
#define active_FH_TUERMODUL_CTRL_copy_IDX 14
#define active_FH_TUERMODUL_CTRL_old_IDX 15
#define active_EINKLEMMSCHUTZ_CTRL_IDX 16
#define active_EINKLEMMSCHUTZ_CTRL_copy_IDX 17
#define active_EINKLEMMSCHUTZ_CTRL_old_IDX 18
#define active_BLOCK_ERKENNUNG_CTRL_IDX 19
#define active_BLOCK_ERKENNUNG_CTRL_copy_IDX 20
#define active_BLOCK_ERKENNUNG_CTRL_old_IDX 21
#define entered_EINSCHALTSTROM_MESSEN_BLOCK_ERKENNUNG_CTRL_IDX 0
#define entered_EINSCHALTSTROM_MESSEN_BLOCK_ERKENNUNG_CTRL_copy_IDX 1
unsigned long
  tm_entered_EINSCHALTSTROM_MESSEN_BLOCK_ERKENNUNG_CTRLch_BLOCK_ERKENNUNG_CTRL__N_copy;
#define entered_WIEDERHOLSPERRE_FH_TUERMODUL_CTRL_IDX 4
#define entered_WIEDERHOLSPERRE_FH_TUERMODUL_CTRL_copy_IDX 5
#define exited_BEREIT_FH_TUERMODUL_CTRL_IDX 6
#define exited_BEREIT_FH_TUERMODUL_CTRL_copy_IDX 7
unsigned long
  tm_entered_WIEDERHOLSPERRE_FH_TUERMODUL_CTRLexited_BEREIT_FH_TUERMODUL_CTRL;
unsigned long tm_entered_WIEDERHOLSPERRE_FH_TUERMODUL_CTRL;
unsigned long sc_FH_TUERMODUL_CTRL_2375_2;
unsigned long sc_FH_TUERMODUL_CTRL_2352_1;
unsigned long sc_FH_TUERMODUL_CTRL_2329_1;
int FH_TUERMODUL_CTRL__N;
int FH_TUERMODUL_CTRL__N_copy;
int FH_TUERMODUL_CTRL__N_old;
unsigned long sc_FH_TUERMODUL_CTRL_1781_10;
unsigned long sc_FH_TUERMODUL_CTRL_1739_10;
float FH_TUERMODUL__POSITION;
float FH_TUERMODUL__I_EIN;
float FH_TUERMODUL__I_EIN_old;
int FH_DU__MFH;
int FH_DU__MFH_copy;
float FH_DU__POSITION;
float FH_DU__I_EIN;
float FH_DU__I_EIN_old;
float BLOCK_ERKENNUNG_CTRL__I_EIN_MAX;
float BLOCK_ERKENNUNG_CTRL__I_EIN_MAX_copy;
int BLOCK_ERKENNUNG_CTRL__N;
int BLOCK_ERKENNUNG_CTRL__N_copy;
int BLOCK_ERKENNUNG_CTRL__N_old;
char FH_TUERMODUL_CTRL__INREVERS2;
char FH_TUERMODUL_CTRL__INREVERS2_copy;
char FH_TUERMODUL_CTRL__INREVERS1;
char FH_TUERMODUL_CTRL__INREVERS1_copy;
char FH_TUERMODUL_CTRL__FT;
char FH_TUERMODUL__SFHZ_ZENTRAL;
char FH_TUERMODUL__SFHZ_ZENTRAL_old;
char FH_TUERMODUL__SFHZ_MEC;
char FH_TUERMODUL__SFHZ_MEC_old;
char FH_TUERMODUL__SFHA_ZENTRAL;
char FH_TUERMODUL__SFHA_ZENTRAL_old;
char FH_TUERMODUL__SFHA_MEC;
char FH_TUERMODUL__SFHA_MEC_old;
char FH_TUERMODUL__KL_50;
char FH_TUERMODUL__BLOCK;
char FH_TUERMODUL__BLOCK_copy;
char FH_TUERMODUL__BLOCK_old;
char FH_TUERMODUL__FT;
char FH_TUERMODUL__SFHZ;
char FH_TUERMODUL__SFHZ_copy;
char FH_TUERMODUL__SFHZ_old;
char FH_TUERMODUL__SFHA;
char FH_TUERMODUL__SFHA_copy;
char FH_TUERMODUL__SFHA_old;
char FH_TUERMODUL__MFHZ;
char FH_TUERMODUL__MFHZ_copy;
char FH_TUERMODUL__MFHZ_old;
char FH_TUERMODUL__MFHA;
char FH_TUERMODUL__MFHA_copy;
char FH_TUERMODUL__MFHA_old;
char FH_TUERMODUL__EKS_LEISTE_AKTIV;
char FH_TUERMODUL__EKS_LEISTE_AKTIV_old;
char FH_TUERMODUL__COM_OPEN;
char FH_TUERMODUL__COM_CLOSE;
char FH_DU__KL_50;
char FH_DU__S_FH_FTZU;
char FH_DU__S_FH_FTAUF;
char FH_DU__FT;
char FH_DU__EKS_LEISTE_AKTIV;
char FH_DU__EKS_LEISTE_AKTIV_old;
char FH_DU__S_FH_TMBFAUFCAN;
char FH_DU__S_FH_TMBFAUFCAN_copy;
char FH_DU__S_FH_TMBFAUFCAN_old;
char FH_DU__S_FH_TMBFZUCAN;
char FH_DU__S_FH_TMBFZUCAN_copy;
char FH_DU__S_FH_TMBFZUCAN_old;
char FH_DU__S_FH_TMBFZUDISC;
char FH_DU__S_FH_TMBFZUDISC_old;
char FH_DU__S_FH_TMBFAUFDISC;
char FH_DU__S_FH_TMBFAUFDISC_old;
char FH_DU__S_FH_ZUDISC;
char FH_DU__S_FH_AUFDISC;
char FH_DU__DOOR_ID;
char FH_DU__BLOCK;
char FH_DU__BLOCK_copy;
char FH_DU__BLOCK_old;
char FH_DU__MFHZ;
char FH_DU__MFHZ_copy;
char FH_DU__MFHZ_old;
char FH_DU__MFHA;
char FH_DU__MFHA_copy;
char FH_DU__MFHA_old;
#define  FH_TUERMODUL_CTRL__END_REVERS_IDX 22
#define  FH_TUERMODUL_CTRL__END_REVERS_copy_IDX 23
#define  FH_TUERMODUL__EINKLEMMUNG_IDX 24

unsigned long time;
char stable;
char step;

char NICHT_INITIALISIERT_NICHT_INITIALISIERT_next_state;  /** 2 bits **/
char ZENTRAL_KINDERSICHERUNG_CTRL_next_state;  /** 1 bits **/
char MEC_KINDERSICHERUNG_CTRL_next_state;  /** 1 bits **/
char KINDERSICHERUNG_CTRL_KINDERSICHERUNG_CTRL_next_state;  /** 2 bits **/
char B_FH_TUERMODUL_CTRL_next_state;  /** 2 bits **/
char A_FH_TUERMODUL_CTRL_next_state;  /** 1 bits **/
char WIEDERHOLSPERRE_FH_TUERMODUL_CTRL_next_state;  /** 1 bits **/
char INITIALISIERT_FH_TUERMODUL_CTRL_next_state;  /** 2 bits **/
char TIPP_SCHLIESSEN_FH_TUERMODUL_CTRL_next_state;  /** 2 bits **/
char MANUELL_SCHLIESSEN_FH_TUERMODUL_CTRL_next_state;  /** 2 bits **/
char OEFFNEN_FH_TUERMODUL_CTRL_next_state;  /** 2 bits **/
char SCHLIESSEN_FH_TUERMODUL_CTRL_next_state;  /** 2 bits **/
char FH_STEUERUNG_DUMMY_FH_STEUERUNG_DUMMY_next_state;	/** 2 bits **/
char EINKLEMMSCHUTZ_CTRL_EINKLEMMSCHUTZ_CTRL_next_state;  /** 2 bits **/
char BEWEGUNG_BLOCK_ERKENNUNG_CTRL_next_state;	/** 2 bits **/
char BLOCK_ERKENNUNG_CTRL_BLOCK_ERKENNUNG_CTRL_next_state;  /** 2 bits **/


void
interface (void)
{
  if (SYS_bit_get (Bitlist, entered_WIEDERHOLSPERRE_FH_TUERMODUL_CTRL_IDX))
    tm_entered_WIEDERHOLSPERRE_FH_TUERMODUL_CTRL = time;
  if (SYS_bit_get (Bitlist, entered_WIEDERHOLSPERRE_FH_TUERMODUL_CTRL_IDX)
      || SYS_bit_get (Bitlist, exited_BEREIT_FH_TUERMODUL_CTRL_IDX))
    tm_entered_WIEDERHOLSPERRE_FH_TUERMODUL_CTRLexited_BEREIT_FH_TUERMODUL_CTRL
      = time;
  if ((sc_FH_TUERMODUL_CTRL_2375_2 != 0)
      && (time - sc_FH_TUERMODUL_CTRL_2375_2 >= 0.5))
    {
      FH_TUERMODUL__MFHA_copy = 0;
      sc_FH_TUERMODUL_CTRL_2375_2 = 0;

    }
  if ((sc_FH_TUERMODUL_CTRL_2352_1 != 0)
      && (time - sc_FH_TUERMODUL_CTRL_2352_1 >= 0.5))
    {
      FH_TUERMODUL__MFHZ_copy = 0;
      sc_FH_TUERMODUL_CTRL_2352_1 = 0;

    }
  if ((sc_FH_TUERMODUL_CTRL_2329_1 != 0)
      && (time - sc_FH_TUERMODUL_CTRL_2329_1 >= 0.5))
    {
      FH_TUERMODUL__MFHZ_copy = 0;
      sc_FH_TUERMODUL_CTRL_2329_1 = 0;

    }
  if ((sc_FH_TUERMODUL_CTRL_1781_10 != 0)
      && (time - sc_FH_TUERMODUL_CTRL_1781_10 >= 0.5))
    {
      sc_FH_TUERMODUL_CTRL_1781_10 = 0;

    }
  if ((sc_FH_TUERMODUL_CTRL_1739_10 != 0)
      && (time - sc_FH_TUERMODUL_CTRL_1739_10 >= 0.5))
    {
      sc_FH_TUERMODUL_CTRL_1739_10 = 0;

    }
  if ((SYS_bit_get
       (Bitlist, entered_EINSCHALTSTROM_MESSEN_BLOCK_ERKENNUNG_CTRL_IDX)
       || BLOCK_ERKENNUNG_CTRL__N != BLOCK_ERKENNUNG_CTRL__N_old))
    tm_entered_EINSCHALTSTROM_MESSEN_BLOCK_ERKENNUNG_CTRLch_BLOCK_ERKENNUNG_CTRL__N_copy
      = time;


}/** interface **/


void
init (void)
{
  tm_entered_EINSCHALTSTROM_MESSEN_BLOCK_ERKENNUNG_CTRLch_BLOCK_ERKENNUNG_CTRL__N_copy
    = 0;
  tm_entered_WIEDERHOLSPERRE_FH_TUERMODUL_CTRLexited_BEREIT_FH_TUERMODUL_CTRL
    = 0;
  tm_entered_WIEDERHOLSPERRE_FH_TUERMODUL_CTRL = 0;
  NICHT_INITIALISIERT_NICHT_INITIALISIERT_next_state = 0;
  ZENTRAL_KINDERSICHERUNG_CTRL_next_state = 0;
  MEC_KINDERSICHERUNG_CTRL_next_state = 0;
  KINDERSICHERUNG_CTRL_KINDERSICHERUNG_CTRL_next_state = 0;
  B_FH_TUERMODUL_CTRL_next_state = 0;
  A_FH_TUERMODUL_CTRL_next_state = 0;
  WIEDERHOLSPERRE_FH_TUERMODUL_CTRL_next_state = 0;
  INITIALISIERT_FH_TUERMODUL_CTRL_next_state = 0;
  TIPP_SCHLIESSEN_FH_TUERMODUL_CTRL_next_state = 0;
  MANUELL_SCHLIESSEN_FH_TUERMODUL_CTRL_next_state = 0;
  OEFFNEN_FH_TUERMODUL_CTRL_next_state = 0;
  SCHLIESSEN_FH_TUERMODUL_CTRL_next_state = 0;
  FH_STEUERUNG_DUMMY_FH_STEUERUNG_DUMMY_next_state = 0;
  EINKLEMMSCHUTZ_CTRL_EINKLEMMSCHUTZ_CTRL_next_state = 0;
  BEWEGUNG_BLOCK_ERKENNUNG_CTRL_next_state = 0;
  BLOCK_ERKENNUNG_CTRL_BLOCK_ERKENNUNG_CTRL_next_state = 0;


}/** init **/



void
generic_KINDERSICHERUNG_CTRL (void)
{
  if (SYS_bit_get (Bitlist, active_KINDERSICHERUNG_CTRL_IDX))
    {
      switch (KINDERSICHERUNG_CTRL_KINDERSICHERUNG_CTRL_next_state)
	{
	case 1:  /** state ZENTRAL in chart KINDERSICHERUNG_CTRL **/
	  {
	    if (!(FH_TUERMODUL__SFHA_ZENTRAL || FH_TUERMODUL__SFHZ_ZENTRAL))
	      {
		stable = 0;
		FH_TUERMODUL__SFHZ_copy = 0;
		FH_TUERMODUL__SFHA_copy = 0;

		KINDERSICHERUNG_CTRL_KINDERSICHERUNG_CTRL_next_state = 3;
		ZENTRAL_KINDERSICHERUNG_CTRL_next_state = 0;
		break;
	      }
	    switch (ZENTRAL_KINDERSICHERUNG_CTRL_next_state)
	      {
	      case 1:	/** state IN_ZENTRAL in chart KINDERSICHERUNG_CTRL **/
		{
		  if ((FH_TUERMODUL__SFHA_ZENTRAL
		       && !(FH_TUERMODUL__SFHA_ZENTRAL_old)))
		    {
		      stable = 0;
		      FH_TUERMODUL__SFHA_copy = 1;

		      ZENTRAL_KINDERSICHERUNG_CTRL_next_state = 1;
		      break;
		    }
		  if ((FH_TUERMODUL__SFHZ_ZENTRAL
		       && !(FH_TUERMODUL__SFHZ_ZENTRAL_old)))
		    {
		      stable = 0;
		      FH_TUERMODUL__SFHZ_copy = 1;

		      ZENTRAL_KINDERSICHERUNG_CTRL_next_state = 1;
		      break;
		    }
		  if ((!(FH_TUERMODUL__SFHA_ZENTRAL)
		       && FH_TUERMODUL__SFHA_ZENTRAL_old))
		    {
		      stable = 0;
		      FH_TUERMODUL__SFHA_copy = 0;

		      ZENTRAL_KINDERSICHERUNG_CTRL_next_state = 1;
		      break;
		    }
		  if ((!(FH_TUERMODUL__SFHZ_ZENTRAL)
		       && FH_TUERMODUL__SFHZ_ZENTRAL_old))
		    {
		      stable = 0;
		      FH_TUERMODUL__SFHZ_copy = 0;

		      ZENTRAL_KINDERSICHERUNG_CTRL_next_state = 1;
		      break;
		    }
		  break;
		}
	      default:
		{
		  stable = 0;
		  break;
		}
	      }
	     /** switch ZENTRAL_KINDERSICHERUNG_CTRL_next_state **/
	    break;
	  }
	case 2:  /** state MEC in chart KINDERSICHERUNG_CTRL **/
	  {
	    if (!(FH_TUERMODUL__SFHA_MEC || FH_TUERMODUL__SFHZ_MEC))
	      {
		stable = 0;
		FH_TUERMODUL__SFHZ_copy = 0;
		FH_TUERMODUL__SFHA_copy = 0;

		KINDERSICHERUNG_CTRL_KINDERSICHERUNG_CTRL_next_state = 3;
		MEC_KINDERSICHERUNG_CTRL_next_state = 0;
		break;
	      }
	    switch (MEC_KINDERSICHERUNG_CTRL_next_state)
	      {
	      case 1:	/** state INMEC in chart KINDERSICHERUNG_CTRL **/
		{
		  if ((FH_TUERMODUL__SFHA_MEC
		       && !(FH_TUERMODUL__SFHA_MEC_old)))
		    {
		      stable = 0;
		      FH_TUERMODUL__SFHA_copy = 1;

		      MEC_KINDERSICHERUNG_CTRL_next_state = 1;
		      break;
		    }
		  if ((FH_TUERMODUL__SFHZ_MEC
		       && !(FH_TUERMODUL__SFHZ_MEC_old)))
		    {
		      stable = 0;
		      FH_TUERMODUL__SFHZ_copy = 1;

		      MEC_KINDERSICHERUNG_CTRL_next_state = 1;
		      break;
		    }
		  if ((!(FH_TUERMODUL__SFHA_MEC)
		       && FH_TUERMODUL__SFHA_MEC_old))
		    {
		      stable = 0;
		      FH_TUERMODUL__SFHA_copy = 0;

		      MEC_KINDERSICHERUNG_CTRL_next_state = 1;
		      break;
		    }
		  if ((!(FH_TUERMODUL__SFHZ_MEC)
		       && FH_TUERMODUL__SFHZ_MEC_old))
		    {
		      stable = 0;
		      FH_TUERMODUL__SFHZ_copy = 0;

		      MEC_KINDERSICHERUNG_CTRL_next_state = 1;
		      break;
		    }
		  break;
		}
	      default:
		{
		  stable = 0;
		  break;
		}
	      }
	     /** switch MEC_KINDERSICHERUNG_CTRL_next_state **/
	    break;
	  }
	case 3:  /** state WAITING in chart KINDERSICHERUNG_CTRL **/
	  {
	    if ((!FH_TUERMODUL__KL_50)
		&& (FH_TUERMODUL__SFHZ_MEC && FH_TUERMODUL__SFHA_MEC))
	      {
		stable = 0;
		FH_TUERMODUL__SFHZ_copy = 1;
		FH_TUERMODUL__SFHA_copy = 1;

		KINDERSICHERUNG_CTRL_KINDERSICHERUNG_CTRL_next_state = 2;
		break;
	      }
	    if ((!FH_TUERMODUL__KL_50)
		&& (FH_TUERMODUL__SFHZ_MEC && !FH_TUERMODUL__SFHA_MEC))
	      {
		stable = 0;
		FH_TUERMODUL__SFHZ_copy = 1;

		KINDERSICHERUNG_CTRL_KINDERSICHERUNG_CTRL_next_state = 2;
		break;
	      }
	    if ((!FH_TUERMODUL__KL_50)
		&& (!FH_TUERMODUL__SFHZ_MEC && FH_TUERMODUL__SFHA_MEC))
	      {
		stable = 0;
		FH_TUERMODUL__SFHA_copy = 1;

		KINDERSICHERUNG_CTRL_KINDERSICHERUNG_CTRL_next_state = 2;
		break;
	      }
	    if ((!FH_TUERMODUL__SFHZ_ZENTRAL && FH_TUERMODUL__SFHA_ZENTRAL
		 && !FH_TUERMODUL__KL_50))
	      {
		stable = 0;
		FH_TUERMODUL__SFHA_copy = 1;

		KINDERSICHERUNG_CTRL_KINDERSICHERUNG_CTRL_next_state = 1;
		break;
	      }
	    if ((FH_TUERMODUL__SFHZ_ZENTRAL && FH_TUERMODUL__SFHA_ZENTRAL))
	      {
		stable = 0;
		FH_TUERMODUL__SFHA_copy = 1;
		FH_TUERMODUL__SFHZ_copy = 1;

		KINDERSICHERUNG_CTRL_KINDERSICHERUNG_CTRL_next_state = 1;
		break;
	      }
	    if ((FH_TUERMODUL__SFHZ_ZENTRAL && !FH_TUERMODUL__SFHA_ZENTRAL
		 && !FH_TUERMODUL__KL_50))
	      {
		stable = 0;
		FH_TUERMODUL__SFHZ_copy = 1;

		KINDERSICHERUNG_CTRL_KINDERSICHERUNG_CTRL_next_state = 1;
		break;
	      }
	    break;
	  }
	default:
	  {
	    stable = 0;
	    KINDERSICHERUNG_CTRL_KINDERSICHERUNG_CTRL_next_state = 3;
	    break;
	  }
	}
       /** switch KINDERSICHERUNG_CTRL_KINDERSICHERUNG_CTRL_next_state **/
    }
}

void
generic_FH_TUERMODUL_CTRL (void)
{
  if (!SYS_bit_get (Bitlist, active_FH_TUERMODUL_CTRL_IDX)
      && SYS_bit_get (Bitlist, active_FH_TUERMODUL_CTRL_old_IDX)
      && !SYS_bit_get (Bitlist, active_FH_TUERMODUL_CTRL_copy_IDX))
    {
      SYS_bit_clr (Bitlist, entered_WIEDERHOLSPERRE_FH_TUERMODUL_CTRL_IDX);
      SYS_bit_clr (Bitlist, exited_BEREIT_FH_TUERMODUL_CTRL_IDX);
    }
  if (SYS_bit_get (Bitlist, active_FH_TUERMODUL_CTRL_IDX))
    {
      if (!SYS_bit_get (Bitlist, active_KINDERSICHERUNG_CTRL_IDX))
	{
	  KINDERSICHERUNG_CTRL_KINDERSICHERUNG_CTRL_next_state = 3;
	}
      SYS_bit_clr (Bitlist, active_KINDERSICHERUNG_CTRL_copy_IDX);
      if (!SYS_bit_get (Bitlist, active_BLOCK_ERKENNUNG_CTRL_IDX))
	{
	  SYS_bit_clr (Bitlist,
		       entered_EINSCHALTSTROM_MESSEN_BLOCK_ERKENNUNG_CTRL_IDX);
	  BLOCK_ERKENNUNG_CTRL_BLOCK_ERKENNUNG_CTRL_next_state = 1;
	}
      SYS_bit_clr (Bitlist, active_BLOCK_ERKENNUNG_CTRL_copy_IDX);
      SYS_bit_set (Bitlist, active_KINDERSICHERUNG_CTRL_copy_IDX);
      SYS_bit_set (Bitlist, active_BLOCK_ERKENNUNG_CTRL_copy_IDX);
      switch (B_FH_TUERMODUL_CTRL_next_state)
	{
	case 1:  /** state ZAEHLER_WHSP_ZU_HOCH in chart FH_TUERMODUL_CTRL **/
	  {
	    if ((FH_TUERMODUL_CTRL__N == 59
		 && !(FH_TUERMODUL_CTRL__N_old == 59)))
	      {
		stable = 0;

		B_FH_TUERMODUL_CTRL_next_state = 3;
		INITIALISIERT_FH_TUERMODUL_CTRL_next_state = 3;
		break;
	      }
	    break;
	  }
	case 2:  /** state NICHT_INITIALISIERT in chart FH_TUERMODUL_CTRL **/
	  {
	    if (((FH_TUERMODUL__BLOCK && !(FH_TUERMODUL__BLOCK_old)))
		&& ((FH_TUERMODUL__MFHZ)))
	      {
		stable = 0;
		FH_TUERMODUL__MFHZ_copy = 0;
		sc_FH_TUERMODUL_CTRL_2329_1 = time;

		B_FH_TUERMODUL_CTRL_next_state = 3;
		INITIALISIERT_FH_TUERMODUL_CTRL_next_state = 3;
		break;
	      }
	    switch (NICHT_INITIALISIERT_NICHT_INITIALISIERT_next_state)
	      {
	      case 1:	/** state SCHLIESSEN in chart NICHT_INITIALISIERT **/
		{
		  if (!(FH_TUERMODUL__SFHZ))
		    {
		      stable = 0;
		      FH_TUERMODUL__MFHZ_copy = 0;

		      NICHT_INITIALISIERT_NICHT_INITIALISIERT_next_state = 3;
		      break;
		    }
		  break;
		}
	      case 2:	/** state OEFFNEN in chart NICHT_INITIALISIERT **/
		{
		  if (!(FH_TUERMODUL__SFHA))
		    {
		      stable = 0;
		      FH_TUERMODUL__MFHA_copy = 0;

		      NICHT_INITIALISIERT_NICHT_INITIALISIERT_next_state = 3;
		      break;
		    }
		  break;
		}
	      case 3:	/** state BEREIT in chart NICHT_INITIALISIERT **/
		{
		  if ((FH_TUERMODUL__SFHA))
		    {
		      stable = 0;
		      FH_TUERMODUL__MFHA_copy = 1;

		      NICHT_INITIALISIERT_NICHT_INITIALISIERT_next_state = 2;
		      break;
		    }
		  if ((FH_TUERMODUL__SFHZ))
		    {
		      stable = 0;
		      FH_TUERMODUL__MFHZ_copy = 1;

		      NICHT_INITIALISIERT_NICHT_INITIALISIERT_next_state = 1;
		      break;
		    }
		  break;
		}
	      default:
		{
		  stable = 0;
		  NICHT_INITIALISIERT_NICHT_INITIALISIERT_next_state = 3;
		  break;
		}
	      }
	     /** switch NICHT_INITIALISIERT_NICHT_INITIALISIERT_next_state **/
	    break;
	  }
	case 3:  /** state INITIALISIERT in chart FH_TUERMODUL_CTRL **/
	  {
	    if (((FH_TUERMODUL_CTRL__N > 60
		  && !(FH_TUERMODUL_CTRL__N_old > 60)))
		&&
		((!(FH_TUERMODUL_CTRL__INREVERS1
		    || FH_TUERMODUL_CTRL__INREVERS2))))
	      {
		stable = 0;
		FH_TUERMODUL__MFHZ_copy = 0;
		FH_TUERMODUL__MFHA_copy = 0;

		B_FH_TUERMODUL_CTRL_next_state = 1;
		break;
	      }
	    if (((FH_TUERMODUL__BLOCK && !(FH_TUERMODUL__BLOCK_old)))
		&& ((FH_TUERMODUL__MFHA)))
	      {
		stable = 0;
		FH_TUERMODUL__MFHA_copy = 0;
		sc_FH_TUERMODUL_CTRL_2375_2 = time;

		B_FH_TUERMODUL_CTRL_next_state = 2;
		NICHT_INITIALISIERT_NICHT_INITIALISIERT_next_state = 3;
		break;
	      }
	    if (((FH_TUERMODUL__BLOCK && !(FH_TUERMODUL__BLOCK_old)))
		&& ((FH_TUERMODUL__MFHZ)))
	      {
		stable = 0;
		FH_TUERMODUL__MFHZ_copy = 0;
		sc_FH_TUERMODUL_CTRL_2352_1 = time;

		B_FH_TUERMODUL_CTRL_next_state = 2;
		NICHT_INITIALISIERT_NICHT_INITIALISIERT_next_state = 3;
		break;
	      }
	    switch (INITIALISIERT_FH_TUERMODUL_CTRL_next_state)
	      {
	      case 1:	/** state OEFFNEN in chart FH_TUERMODUL_CTRL **/
		{
		  if ((FH_TUERMODUL__POSITION >= 405))
		    {
		      stable = 0;
		      FH_TUERMODUL__MFHA_copy = 0;

		      INITIALISIERT_FH_TUERMODUL_CTRL_next_state = 3;
		      break;
		    }
		  switch (OEFFNEN_FH_TUERMODUL_CTRL_next_state)
		    {
		    case 1:  /** state TIPP_OEFFNEN in chart FH_TUERMODUL_CTRL **/
		      {
			if ((FH_TUERMODUL__SFHZ && !(FH_TUERMODUL__SFHZ_old))
			    || (FH_TUERMODUL__SFHA
				&& !(FH_TUERMODUL__SFHA_old)))
			  {
			    stable = 0;
			    FH_TUERMODUL__MFHA_copy = 0;

			    INITIALISIERT_FH_TUERMODUL_CTRL_next_state = 3;
			    OEFFNEN_FH_TUERMODUL_CTRL_next_state = 0;
			    break;
			  }
			break;
		      }
		    case 2:  /** state MAN_OEFFNEN in chart FH_TUERMODUL_CTRL **/
		      {
			if ((FH_TUERMODUL__SFHZ && !(FH_TUERMODUL__SFHZ_old)))
			  {
			    stable = 0;

			    OEFFNEN_FH_TUERMODUL_CTRL_next_state = 1;
			    break;
			  }
			if ((!(FH_TUERMODUL__SFHA) && FH_TUERMODUL__SFHA_old))
			  {
			    stable = 0;
			    FH_TUERMODUL__MFHA_copy = 0;

			    INITIALISIERT_FH_TUERMODUL_CTRL_next_state = 3;
			    OEFFNEN_FH_TUERMODUL_CTRL_next_state = 0;
			    break;
			  }
			break;
		      }
		    default:
		      {
			stable = 0;
			OEFFNEN_FH_TUERMODUL_CTRL_next_state = 2;
			break;
		      }
		    }
		   /** switch OEFFNEN_FH_TUERMODUL_CTRL_next_state **/
		  break;
		}
	      case 2:	/** state SCHLIESSEN in chart FH_TUERMODUL_CTRL **/
		{
		  if ((FH_TUERMODUL__POSITION <= 0))
		    {
		      stable = 0;
		      FH_TUERMODUL__MFHZ_copy = 0;

		      INITIALISIERT_FH_TUERMODUL_CTRL_next_state = 3;
		      break;
		    }
		  switch (SCHLIESSEN_FH_TUERMODUL_CTRL_next_state)
		    {
		    case 1:  /** state TIPP_SCHLIESSEN in chart FH_TUERMODUL_CTRL **/
		      {
			if ((FH_TUERMODUL__SFHA && !(FH_TUERMODUL__SFHA_old))
			    || (FH_TUERMODUL__SFHZ
				&& !(FH_TUERMODUL__SFHZ_old)))
			  {
			    stable = 0;
			    FH_TUERMODUL__MFHZ_copy = 0;

			    INITIALISIERT_FH_TUERMODUL_CTRL_next_state = 3;
			    break;
			  }
			switch (TIPP_SCHLIESSEN_FH_TUERMODUL_CTRL_next_state)
			  {
			  case 1:  /** state REVERSIEREN2 in chart FH_TUERMODUL_CTRL **/
			    {
			      SYS_bit_clr (Bitlist,
					   FH_TUERMODUL_CTRL__END_REVERS_copy_IDX);
			      if (SYS_bit_get
				  (Bitlist,
				   FH_TUERMODUL_CTRL__END_REVERS_IDX))
				{
				  stable = 0;
				  FH_TUERMODUL__MFHZ_copy = 1;
				  FH_TUERMODUL_CTRL__INREVERS2_copy = 0;

				  TIPP_SCHLIESSEN_FH_TUERMODUL_CTRL_next_state
				    = 2;
				  FH_TUERMODUL__MFHA_copy = 0;

				  SYS_bit_set (Bitlist,
					       active_EINKLEMMSCHUTZ_CTRL_copy_IDX);
				  break;
				}
			      break;
			    }
			  case 2:  /** state TIPP_SCHLIESSEN1 in chart FH_TUERMODUL_CTRL **/
			    {
			      if (SYS_bit_get
				  (Bitlist, FH_TUERMODUL__EINKLEMMUNG_IDX))
				{
				  stable = 0;
				  FH_TUERMODUL_CTRL__INREVERS2_copy = 1;

				  SYS_bit_set (Bitlist,
					       FH_TUERMODUL_CTRL__END_REVERS_copy_IDX);
				  TIPP_SCHLIESSEN_FH_TUERMODUL_CTRL_next_state
				    = 1;
				  SYS_bit_clr (Bitlist,
					       active_EINKLEMMSCHUTZ_CTRL_copy_IDX);
				  FH_TUERMODUL__MFHZ_copy = 0;

				  sc_FH_TUERMODUL_CTRL_1781_10 = time;
				  FH_TUERMODUL__MFHA_copy = 1;
				  break;
				}
			      break;
			    }
			  default:
			    {
			      stable = 0;
			      TIPP_SCHLIESSEN_FH_TUERMODUL_CTRL_next_state =
				2;
			      SYS_bit_set (Bitlist,
					   active_EINKLEMMSCHUTZ_CTRL_copy_IDX);
			      break;
			    }
			  }
			 /** switch TIPP_SCHLIESSEN_FH_TUERMODUL_CTRL_next_state **/
			break;
		      }
		    case 2:  /** state MANUELL_SCHLIESSEN in chart FH_TUERMODUL_CTRL **/
		      {
			if ((!(FH_TUERMODUL__SFHZ) && FH_TUERMODUL__SFHZ_old))
			  {
			    stable = 0;
			    FH_TUERMODUL__MFHZ_copy = 0;

			    INITIALISIERT_FH_TUERMODUL_CTRL_next_state = 3;
			    break;
			  }
			switch
			  (MANUELL_SCHLIESSEN_FH_TUERMODUL_CTRL_next_state)
			  {
			  case 1:  /** state REVERSIEREN1 in chart FH_TUERMODUL_CTRL **/
			    {
			      SYS_bit_clr (Bitlist,
					   FH_TUERMODUL_CTRL__END_REVERS_copy_IDX);
			      if (SYS_bit_get
				  (Bitlist,
				   FH_TUERMODUL_CTRL__END_REVERS_IDX))
				{
				  stable = 0;
				  FH_TUERMODUL_CTRL__INREVERS1_copy = 0;

				  MANUELL_SCHLIESSEN_FH_TUERMODUL_CTRL_next_state
				    = 2;
				  FH_TUERMODUL__MFHA_copy = 0;

				  SYS_bit_set (Bitlist,
					       active_EINKLEMMSCHUTZ_CTRL_copy_IDX);
				  FH_TUERMODUL__MFHZ_copy = 1;
				  break;
				}
			      break;
			    }
			  case 2:  /** state MAN_SCHLIESSEN in chart FH_TUERMODUL_CTRL **/
			    {
			      if (SYS_bit_get
				  (Bitlist, FH_TUERMODUL__EINKLEMMUNG_IDX))
				{
				  stable = 0;
				  FH_TUERMODUL__MFHZ_copy = 0;
				  FH_TUERMODUL_CTRL__INREVERS1_copy = 1;

				  SYS_bit_set (Bitlist,
					       FH_TUERMODUL_CTRL__END_REVERS_copy_IDX);
				  MANUELL_SCHLIESSEN_FH_TUERMODUL_CTRL_next_state
				    = 1;
				  SYS_bit_clr (Bitlist,
					       active_EINKLEMMSCHUTZ_CTRL_copy_IDX);

				  sc_FH_TUERMODUL_CTRL_1739_10 = time;
				  FH_TUERMODUL__MFHA_copy = 1;
				  break;
				}
			      if ((FH_TUERMODUL__SFHA
				   && !(FH_TUERMODUL__SFHA_old)))
				{
				  stable = 0;

				  SCHLIESSEN_FH_TUERMODUL_CTRL_next_state = 1;
				  MANUELL_SCHLIESSEN_FH_TUERMODUL_CTRL_next_state
				    = 0;
				  break;
				}
			      break;
			    }
			  default:
			    {
			      stable = 0;
			      MANUELL_SCHLIESSEN_FH_TUERMODUL_CTRL_next_state
				= 2;
			      SYS_bit_set (Bitlist,
					   active_EINKLEMMSCHUTZ_CTRL_copy_IDX);
			      FH_TUERMODUL__MFHZ_copy = 1;
			      break;
			    }
			  }
			 /** switch MANUELL_SCHLIESSEN_FH_TUERMODUL_CTRL_next_state **/
			break;
		      }
		    default:
		      {
			stable = 0;
			SCHLIESSEN_FH_TUERMODUL_CTRL_next_state = 2;
			MANUELL_SCHLIESSEN_FH_TUERMODUL_CTRL_next_state = 2;
			SYS_bit_set (Bitlist,
				     active_EINKLEMMSCHUTZ_CTRL_copy_IDX);
			FH_TUERMODUL__MFHZ_copy = 1;
			break;
		      }
		    }
		   /** switch SCHLIESSEN_FH_TUERMODUL_CTRL_next_state **/
		  break;
		}
	      case 3:	/** state BEREIT in chart FH_TUERMODUL_CTRL **/
		{
		  if (((FH_TUERMODUL__SFHZ && !(FH_TUERMODUL__SFHZ_old)))
		      && ((FH_TUERMODUL__POSITION > 0)))
		    {
		      stable = 0;

		      INITIALISIERT_FH_TUERMODUL_CTRL_next_state = 2;
		      SCHLIESSEN_FH_TUERMODUL_CTRL_next_state = 2;
		      MANUELL_SCHLIESSEN_FH_TUERMODUL_CTRL_next_state = 2;
		      SYS_bit_set (Bitlist,
				   active_EINKLEMMSCHUTZ_CTRL_copy_IDX);
		      FH_TUERMODUL__MFHZ_copy = 1;
		      break;
		    }
		  if (((FH_TUERMODUL__SFHA && !(FH_TUERMODUL__SFHA_old)))
		      && ((FH_TUERMODUL__POSITION < 405)))
		    {
		      stable = 0;
		      FH_TUERMODUL__MFHA_copy = 1;

		      INITIALISIERT_FH_TUERMODUL_CTRL_next_state = 1;
		      OEFFNEN_FH_TUERMODUL_CTRL_next_state = 2;
		      break;
		    }
		  break;
		}
	      default:
		{
		  stable = 0;
		  INITIALISIERT_FH_TUERMODUL_CTRL_next_state = 3;
		  break;
		}
	      }
	     /** switch INITIALISIERT_FH_TUERMODUL_CTRL_next_state **/
	    break;
	  }
	default:
	  {
	    stable = 0;
	    B_FH_TUERMODUL_CTRL_next_state = 2;
	    break;
	  }
	}
       /** switch B_FH_TUERMODUL_CTRL_next_state **/
      switch (A_FH_TUERMODUL_CTRL_next_state)
	{
	case 1:  /** state WIEDERHOLSPERRE in chart FH_TUERMODUL_CTRL **/
	  {
	    SYS_bit_clr (Bitlist,
			 entered_WIEDERHOLSPERRE_FH_TUERMODUL_CTRL_copy_IDX);
	    if ((step == 1
		 &&
		 tm_entered_WIEDERHOLSPERRE_FH_TUERMODUL_CTRLexited_BEREIT_FH_TUERMODUL_CTRL
		 != 0
		 && (time -
		     tm_entered_WIEDERHOLSPERRE_FH_TUERMODUL_CTRLexited_BEREIT_FH_TUERMODUL_CTRL
		     == 1)) && ((FH_TUERMODUL__MFHZ || FH_TUERMODUL__MFHA)))
	      {
		stable = 0;
		FH_TUERMODUL_CTRL__N = FH_TUERMODUL_CTRL__N + 1;

		A_FH_TUERMODUL_CTRL_next_state = 1;
		SYS_bit_set (Bitlist,
			     entered_WIEDERHOLSPERRE_FH_TUERMODUL_CTRL_copy_IDX);
		WIEDERHOLSPERRE_FH_TUERMODUL_CTRL_next_state = 1;
		break;
	      }
	    switch (WIEDERHOLSPERRE_FH_TUERMODUL_CTRL_next_state)
	      {
	      case 1:	/** state WDHSP in chart FH_TUERMODUL_CTRL **/
		{
		  if ((step == 1
		       && tm_entered_WIEDERHOLSPERRE_FH_TUERMODUL_CTRL != 0
		       && (time -
			   tm_entered_WIEDERHOLSPERRE_FH_TUERMODUL_CTRL == 3))
		      &&
		      (((!(FH_TUERMODUL__MFHZ || FH_TUERMODUL__MFHA))
			&& FH_TUERMODUL_CTRL__N > 0)))
		    {
		      stable = 0;
		      FH_TUERMODUL_CTRL__N = FH_TUERMODUL_CTRL__N - 1;

		      WIEDERHOLSPERRE_FH_TUERMODUL_CTRL_next_state = 1;
		      break;
		    }
		  break;
		}
	      default:
		{
		  stable = 0;
		  SYS_bit_set (Bitlist,
			       entered_WIEDERHOLSPERRE_FH_TUERMODUL_CTRL_copy_IDX);
		  WIEDERHOLSPERRE_FH_TUERMODUL_CTRL_next_state = 1;
		  break;
		}
	      }
	     /** switch WIEDERHOLSPERRE_FH_TUERMODUL_CTRL_next_state **/
	    break;
	  }
	default:
	  {
	    stable = 0;
	    FH_TUERMODUL_CTRL__N = 0;
	    A_FH_TUERMODUL_CTRL_next_state = 1;
	    SYS_bit_set (Bitlist,
			 entered_WIEDERHOLSPERRE_FH_TUERMODUL_CTRL_copy_IDX);
	    WIEDERHOLSPERRE_FH_TUERMODUL_CTRL_next_state = 1;
	    break;
	  }
	}
       /** switch A_FH_TUERMODUL_CTRL_next_state **/
      SYS_bit_cpy (Bitlist,
		   entered_WIEDERHOLSPERRE_FH_TUERMODUL_CTRL_copy_IDX,
		   Bitlist, entered_WIEDERHOLSPERRE_FH_TUERMODUL_CTRL_IDX);
      SYS_bit_cpy (Bitlist, exited_BEREIT_FH_TUERMODUL_CTRL_copy_IDX, Bitlist,
		   exited_BEREIT_FH_TUERMODUL_CTRL_IDX);
    }
}

void
generic_EINKLEMMSCHUTZ_CTRL (void)
{
  if (SYS_bit_get (Bitlist, active_EINKLEMMSCHUTZ_CTRL_IDX))
    {
      switch (EINKLEMMSCHUTZ_CTRL_EINKLEMMSCHUTZ_CTRL_next_state)
	{
	case 1:  /** state NORMALBETRIEB in chart EINKLEMMSCHUTZ_CTRL **/
	  {
	    if (((FH_TUERMODUL__EKS_LEISTE_AKTIV
		  && !(FH_TUERMODUL__EKS_LEISTE_AKTIV_old)))
		&& ((!(FH_TUERMODUL__SFHZ && FH_TUERMODUL__SFHA))))
	      {
		stable = 0;

		SYS_bit_set (Bitlist, FH_TUERMODUL__EINKLEMMUNG_IDX);
		EINKLEMMSCHUTZ_CTRL_EINKLEMMSCHUTZ_CTRL_next_state = 2;
		break;
	      }
	    break;
	  }
	case 2:  /** state EINKLEMMUNG in chart EINKLEMMSCHUTZ_CTRL **/
	  {
	    SYS_bit_clr (Bitlist, FH_TUERMODUL__EINKLEMMUNG_IDX);
	    if ((!(FH_TUERMODUL__EKS_LEISTE_AKTIV)
		 && FH_TUERMODUL__EKS_LEISTE_AKTIV_old))
	      {
		stable = 0;

		EINKLEMMSCHUTZ_CTRL_EINKLEMMSCHUTZ_CTRL_next_state = 1;
		break;
	      }
	    break;
	  }
	default:
	  {
	    stable = 0;
	    EINKLEMMSCHUTZ_CTRL_EINKLEMMSCHUTZ_CTRL_next_state = 1;
	    break;
	  }
	}
       /** switch EINKLEMMSCHUTZ_CTRL_EINKLEMMSCHUTZ_CTRL_next_state **/
    }
}

void
generic_BLOCK_ERKENNUNG_CTRL (void)
{
  if (!SYS_bit_get (Bitlist, active_BLOCK_ERKENNUNG_CTRL_IDX)
      && SYS_bit_get (Bitlist, active_BLOCK_ERKENNUNG_CTRL_old_IDX)
      && !SYS_bit_get (Bitlist, active_BLOCK_ERKENNUNG_CTRL_copy_IDX))
    {
      SYS_bit_clr (Bitlist,
		   entered_EINSCHALTSTROM_MESSEN_BLOCK_ERKENNUNG_CTRL_IDX);
    }
  if (SYS_bit_get (Bitlist, active_BLOCK_ERKENNUNG_CTRL_IDX))
    {
      switch (BLOCK_ERKENNUNG_CTRL_BLOCK_ERKENNUNG_CTRL_next_state)
	{
	case 1:  /** state KEINE_BEWEGUNG in chart BLOCK_ERKENNUNG_CTRL **/
	  {
	    if ((FH_TUERMODUL__I_EIN != FH_TUERMODUL__I_EIN_old)
		&& ((FH_TUERMODUL__I_EIN > 0)))
	      {
		stable = 0;
		FH_TUERMODUL__BLOCK_copy = 0;

		BLOCK_ERKENNUNG_CTRL_BLOCK_ERKENNUNG_CTRL_next_state = 2;
		BLOCK_ERKENNUNG_CTRL__N = 0;
		BLOCK_ERKENNUNG_CTRL__I_EIN_MAX = 2;
		BEWEGUNG_BLOCK_ERKENNUNG_CTRL_next_state = 3;
		SYS_bit_set (Bitlist,
			     entered_EINSCHALTSTROM_MESSEN_BLOCK_ERKENNUNG_CTRL_IDX);
		break;
	      }
	    break;
	  }
	case 2:  /** state BEWEGUNG in chart BLOCK_ERKENNUNG_CTRL **/
	  {
	    if ((!(FH_TUERMODUL__MFHA) && FH_TUERMODUL__MFHA_old)
		|| (!(FH_TUERMODUL__MFHZ) && FH_TUERMODUL__MFHZ_old))
	      {
		stable = 0;

		BLOCK_ERKENNUNG_CTRL_BLOCK_ERKENNUNG_CTRL_next_state = 1;
		BEWEGUNG_BLOCK_ERKENNUNG_CTRL_next_state = 0;
		break;
	      }
	    switch (BEWEGUNG_BLOCK_ERKENNUNG_CTRL_next_state)
	      {
	      case 1:	/** state FENSTER_BLOCKIERT in chart BLOCK_ERKENNUNG_CTRL **/
		{
		  break;
		}
	      case 2:	/** state FENSTER_BEWEGT_SICH in chart BLOCK_ERKENNUNG_CTRL **/
		{
		  if ((FH_TUERMODUL__I_EIN >
		       (BLOCK_ERKENNUNG_CTRL__I_EIN_MAX - 2)))
		    {
		      stable = 0;
		      FH_TUERMODUL__BLOCK_copy = 1;

		      BEWEGUNG_BLOCK_ERKENNUNG_CTRL_next_state = 1;
		      break;
		    }
		  break;
		}
	      case 3:	/** state EINSCHALTSTROM_MESSEN in chart BLOCK_ERKENNUNG_CTRL **/
		{
		  SYS_bit_clr (Bitlist,
			       entered_EINSCHALTSTROM_MESSEN_BLOCK_ERKENNUNG_CTRL_IDX);
		  if ((BLOCK_ERKENNUNG_CTRL__N == 11
		       && !(BLOCK_ERKENNUNG_CTRL__N_old == 11)))
		    {
		      stable = 0;

		      BEWEGUNG_BLOCK_ERKENNUNG_CTRL_next_state = 2;
		      break;
		    }
		  /** static reactions: **/
		  if (BEWEGUNG_BLOCK_ERKENNUNG_CTRL_next_state == 3)
		    {
		      if (step == 1
			  &&
			  tm_entered_EINSCHALTSTROM_MESSEN_BLOCK_ERKENNUNG_CTRLch_BLOCK_ERKENNUNG_CTRL__N_copy
			  != 0
			  && (time -
			      tm_entered_EINSCHALTSTROM_MESSEN_BLOCK_ERKENNUNG_CTRLch_BLOCK_ERKENNUNG_CTRL__N_copy
			      == 0.002))
			{
			  BLOCK_ERKENNUNG_CTRL__N =
			    BLOCK_ERKENNUNG_CTRL__N + 1;
			  if ((FH_TUERMODUL__I_EIN >
			       BLOCK_ERKENNUNG_CTRL__I_EIN_MAX))
			    {
			      BLOCK_ERKENNUNG_CTRL__I_EIN_MAX =
				FH_TUERMODUL__I_EIN;

			    }

			}
		    }
		  /** end static reactions **/
		  break;
		}
	      default:
		{
		  stable = 0;
		  BLOCK_ERKENNUNG_CTRL__N = 0;
		  BLOCK_ERKENNUNG_CTRL__I_EIN_MAX = 2;
		  BEWEGUNG_BLOCK_ERKENNUNG_CTRL_next_state = 3;
		  SYS_bit_set (Bitlist,
			       entered_EINSCHALTSTROM_MESSEN_BLOCK_ERKENNUNG_CTRL_IDX);
		  break;
		}
	      }
	     /** switch BEWEGUNG_BLOCK_ERKENNUNG_CTRL_next_state **/
	    break;
	  }
	default:
	  {
	    stable = 0;
	    BLOCK_ERKENNUNG_CTRL_BLOCK_ERKENNUNG_CTRL_next_state = 1;
	    break;
	  }
	}
       /** switch BLOCK_ERKENNUNG_CTRL_BLOCK_ERKENNUNG_CTRL_next_state **/
    }
}



void
FH_DU (void)
{
  time = 1;   /**SYS_get_clock()**/
  stable = 0;
  step = 0;
  while (!stable)
    {
      stable = 1;
      step++;
      {
	switch (FH_STEUERUNG_DUMMY_FH_STEUERUNG_DUMMY_next_state)
	  {
	  case 1:   /** state SCHLIESSEN in chart FH_STEUERUNG_DUMMY **/
	    {
	      if ((!(FH_DU__MFHZ) && FH_DU__MFHZ_old))
		{
		  stable = 0;
		  FH_DU__MFH = 0;

		  FH_STEUERUNG_DUMMY_FH_STEUERUNG_DUMMY_next_state = 2;
		  break;
		}
	      break;
	    }
	  case 2:   /** state BEREIT in chart FH_STEUERUNG_DUMMY **/
	    {
	      if ((FH_DU__MFHZ && !(FH_DU__MFHZ_old)))
		{
		  stable = 0;
		  FH_DU__MFH = -100;

		  FH_STEUERUNG_DUMMY_FH_STEUERUNG_DUMMY_next_state = 1;
		  break;
		}
	      if ((FH_DU__MFHA && !(FH_DU__MFHA_old)))
		{
		  stable = 0;
		  FH_DU__MFH = 100;

		  FH_STEUERUNG_DUMMY_FH_STEUERUNG_DUMMY_next_state = 3;
		  break;
		}
	      break;
	    }
	  case 3:   /** state OEFFNEN in chart FH_STEUERUNG_DUMMY **/
	    {
	      if ((!(FH_DU__MFHA) && FH_DU__MFHA_old))
		{
		  stable = 0;
		  FH_DU__MFH = 0;

		  FH_STEUERUNG_DUMMY_FH_STEUERUNG_DUMMY_next_state = 2;
		  break;
		}
	      break;
	    }
	  default:
	    {
	      stable = 0;
	      FH_DU__MFH = 0;
	      FH_STEUERUNG_DUMMY_FH_STEUERUNG_DUMMY_next_state = 2;
	      break;
	    }
	  }
	  /** switch FH_STEUERUNG_DUMMY_FH_STEUERUNG_DUMMY_next_state **/
      }
      {
	{
	  if (!SYS_bit_get (Bitlist, active_KINDERSICHERUNG_CTRL_IDX))
	    {
	      KINDERSICHERUNG_CTRL_KINDERSICHERUNG_CTRL_next_state = 3;
	    }
	  SYS_bit_clr (Bitlist, active_KINDERSICHERUNG_CTRL_copy_IDX);
	  if (!SYS_bit_get (Bitlist, active_EINKLEMMSCHUTZ_CTRL_IDX))
	    {
	      EINKLEMMSCHUTZ_CTRL_EINKLEMMSCHUTZ_CTRL_next_state = 1;
	    }
	  SYS_bit_clr (Bitlist, active_EINKLEMMSCHUTZ_CTRL_copy_IDX);
	  if (!SYS_bit_get (Bitlist, active_BLOCK_ERKENNUNG_CTRL_IDX))
	    {
	      SYS_bit_clr (Bitlist,
			   entered_EINSCHALTSTROM_MESSEN_BLOCK_ERKENNUNG_CTRL_IDX);
	      BLOCK_ERKENNUNG_CTRL_BLOCK_ERKENNUNG_CTRL_next_state = 1;
	    }
	  SYS_bit_clr (Bitlist, active_BLOCK_ERKENNUNG_CTRL_copy_IDX);
	  if (!SYS_bit_get (Bitlist, active_FH_TUERMODUL_CTRL_IDX))
	    {
	      SYS_bit_clr (Bitlist,
			   entered_WIEDERHOLSPERRE_FH_TUERMODUL_CTRL_IDX);
	      SYS_bit_clr (Bitlist, exited_BEREIT_FH_TUERMODUL_CTRL_IDX);
	      B_FH_TUERMODUL_CTRL_next_state = 2;
	      FH_TUERMODUL_CTRL__N = 0;
	      A_FH_TUERMODUL_CTRL_next_state = 1;
	      SYS_bit_set (Bitlist,
			   entered_WIEDERHOLSPERRE_FH_TUERMODUL_CTRL_copy_IDX);
	      WIEDERHOLSPERRE_FH_TUERMODUL_CTRL_next_state = 1;
	    }
	  SYS_bit_clr (Bitlist, active_FH_TUERMODUL_CTRL_copy_IDX);
	  SYS_bit_set (Bitlist, active_KINDERSICHERUNG_CTRL_copy_IDX);
	  SYS_bit_set (Bitlist, active_EINKLEMMSCHUTZ_CTRL_copy_IDX);
	  SYS_bit_set (Bitlist, active_BLOCK_ERKENNUNG_CTRL_copy_IDX);
	  SYS_bit_set (Bitlist, active_FH_TUERMODUL_CTRL_copy_IDX);
	    /** static reactions: **/
	  if (FH_DU__S_FH_TMBFZUCAN != FH_DU__S_FH_TMBFZUCAN_old)
	    {
	      if ((!FH_DU__DOOR_ID))
		{
		  FH_DU__S_FH_FTZU = FH_DU__S_FH_TMBFZUCAN;

		}

	    }
	  if (FH_DU__S_FH_TMBFZUDISC != FH_DU__S_FH_TMBFZUDISC_old)
	    {
	      if (FH_DU__DOOR_ID)
		{
		  FH_DU__S_FH_TMBFZUCAN = FH_DU__S_FH_TMBFZUDISC;

		}

	    }
	  if (FH_DU__S_FH_TMBFAUFCAN != FH_DU__S_FH_TMBFAUFCAN_old)
	    {
	      if ((!FH_DU__DOOR_ID))
		{
		  FH_DU__S_FH_FTAUF = FH_DU__S_FH_TMBFAUFCAN;

		}

	    }
	  if (FH_DU__S_FH_TMBFAUFDISC != FH_DU__S_FH_TMBFAUFDISC_old)
	    {
	      if (FH_DU__DOOR_ID)
		{
		  FH_DU__S_FH_TMBFAUFCAN = FH_DU__S_FH_TMBFAUFDISC;

		}

	    }
	    /** end static reactions **/
	}
      }
      SYS_bit_cpy (Bitlist, active_KINDERSICHERUNG_CTRL_IDX, Bitlist,
		   active_KINDERSICHERUNG_CTRL_old_IDX);
      SYS_bit_cpy (Bitlist, active_FH_TUERMODUL_CTRL_IDX, Bitlist,
		   active_FH_TUERMODUL_CTRL_old_IDX);
      SYS_bit_cpy (Bitlist, active_EINKLEMMSCHUTZ_CTRL_IDX, Bitlist,
		   active_EINKLEMMSCHUTZ_CTRL_old_IDX);
      SYS_bit_cpy (Bitlist, active_BLOCK_ERKENNUNG_CTRL_IDX, Bitlist,
		   active_BLOCK_ERKENNUNG_CTRL_old_IDX);
      FH_TUERMODUL__SFHA_MEC = FH_DU__S_FH_AUFDISC;
      FH_TUERMODUL__SFHA_ZENTRAL = FH_DU__S_FH_FTAUF;
      FH_TUERMODUL__SFHZ_MEC = FH_DU__S_FH_ZUDISC;
      FH_TUERMODUL__SFHZ_ZENTRAL = FH_DU__S_FH_FTZU;

      generic_KINDERSICHERUNG_CTRL ();

      FH_DU__MFHA = FH_TUERMODUL__MFHA;
      FH_DU__MFHZ = FH_TUERMODUL__MFHZ;
      FH_DU__I_EIN = FH_TUERMODUL__I_EIN;
      FH_DU__EKS_LEISTE_AKTIV = FH_TUERMODUL__EKS_LEISTE_AKTIV;
      FH_DU__POSITION = FH_TUERMODUL__POSITION;
      FH_DU__FT = FH_TUERMODUL__FT;
      FH_DU__S_FH_AUFDISC = FH_TUERMODUL__SFHA_MEC;
      FH_DU__S_FH_FTAUF = FH_TUERMODUL__SFHA_ZENTRAL;
      FH_DU__S_FH_ZUDISC = FH_TUERMODUL__SFHZ_MEC;
      FH_DU__S_FH_FTZU = FH_TUERMODUL__SFHZ_ZENTRAL;
      FH_DU__KL_50 = FH_TUERMODUL__KL_50;
      FH_DU__BLOCK = FH_TUERMODUL__BLOCK;

      FH_TUERMODUL__SFHA_MEC = FH_DU__S_FH_AUFDISC;
      FH_TUERMODUL__SFHA_ZENTRAL = FH_DU__S_FH_FTAUF;
      FH_TUERMODUL__SFHZ_MEC = FH_DU__S_FH_ZUDISC;
      FH_TUERMODUL__SFHZ_ZENTRAL = FH_DU__S_FH_FTZU;

      generic_FH_TUERMODUL_CTRL ();

      FH_DU__MFHA = FH_TUERMODUL__MFHA;
      FH_DU__MFHZ = FH_TUERMODUL__MFHZ;
      FH_DU__I_EIN = FH_TUERMODUL__I_EIN;
      FH_DU__EKS_LEISTE_AKTIV = FH_TUERMODUL__EKS_LEISTE_AKTIV;
      FH_DU__POSITION = FH_TUERMODUL__POSITION;
      FH_DU__FT = FH_TUERMODUL__FT;
      FH_DU__S_FH_AUFDISC = FH_TUERMODUL__SFHA_MEC;
      FH_DU__S_FH_FTAUF = FH_TUERMODUL__SFHA_ZENTRAL;
      FH_DU__S_FH_ZUDISC = FH_TUERMODUL__SFHZ_MEC;
      FH_DU__S_FH_FTZU = FH_TUERMODUL__SFHZ_ZENTRAL;
      FH_DU__KL_50 = FH_TUERMODUL__KL_50;
      FH_DU__BLOCK = FH_TUERMODUL__BLOCK;

      FH_TUERMODUL__SFHA_MEC = FH_DU__S_FH_AUFDISC;
      FH_TUERMODUL__SFHA_ZENTRAL = FH_DU__S_FH_FTAUF;
      FH_TUERMODUL__SFHZ_MEC = FH_DU__S_FH_ZUDISC;
      FH_TUERMODUL__SFHZ_ZENTRAL = FH_DU__S_FH_FTZU;

      generic_EINKLEMMSCHUTZ_CTRL ();

      FH_DU__MFHA = FH_TUERMODUL__MFHA;
      FH_DU__MFHZ = FH_TUERMODUL__MFHZ;
      FH_DU__I_EIN = FH_TUERMODUL__I_EIN;
      FH_DU__EKS_LEISTE_AKTIV = FH_TUERMODUL__EKS_LEISTE_AKTIV;
      FH_DU__POSITION = FH_TUERMODUL__POSITION;
      FH_DU__FT = FH_TUERMODUL__FT;
      FH_DU__S_FH_AUFDISC = FH_TUERMODUL__SFHA_MEC;
      FH_DU__S_FH_FTAUF = FH_TUERMODUL__SFHA_ZENTRAL;
      FH_DU__S_FH_ZUDISC = FH_TUERMODUL__SFHZ_MEC;
      FH_DU__S_FH_FTZU = FH_TUERMODUL__SFHZ_ZENTRAL;
      FH_DU__KL_50 = FH_TUERMODUL__KL_50;
      FH_DU__BLOCK = FH_TUERMODUL__BLOCK;

      FH_TUERMODUL__SFHA_MEC = FH_DU__S_FH_AUFDISC;
      FH_TUERMODUL__SFHA_ZENTRAL = FH_DU__S_FH_FTAUF;
      FH_TUERMODUL__SFHZ_MEC = FH_DU__S_FH_ZUDISC;
      FH_TUERMODUL__SFHZ_ZENTRAL = FH_DU__S_FH_FTZU;

      generic_BLOCK_ERKENNUNG_CTRL ();

      FH_DU__MFHA = FH_TUERMODUL__MFHA;
      FH_DU__MFHZ = FH_TUERMODUL__MFHZ;
      FH_DU__I_EIN = FH_TUERMODUL__I_EIN;
      FH_DU__EKS_LEISTE_AKTIV = FH_TUERMODUL__EKS_LEISTE_AKTIV;
      FH_DU__POSITION = FH_TUERMODUL__POSITION;
      FH_DU__FT = FH_TUERMODUL__FT;
      FH_DU__S_FH_AUFDISC = FH_TUERMODUL__SFHA_MEC;
      FH_DU__S_FH_FTAUF = FH_TUERMODUL__SFHA_ZENTRAL;
      FH_DU__S_FH_ZUDISC = FH_TUERMODUL__SFHZ_MEC;
      FH_DU__S_FH_FTZU = FH_TUERMODUL__SFHZ_ZENTRAL;
      FH_DU__KL_50 = FH_TUERMODUL__KL_50;
      FH_DU__BLOCK = FH_TUERMODUL__BLOCK;

      SYS_bit_cpy (Bitlist, active_KINDERSICHERUNG_CTRL_copy_IDX, Bitlist,
		   active_KINDERSICHERUNG_CTRL_IDX);
      SYS_bit_cpy (Bitlist, active_FH_TUERMODUL_CTRL_copy_IDX, Bitlist,
		   active_FH_TUERMODUL_CTRL_IDX);
      SYS_bit_cpy (Bitlist, active_EINKLEMMSCHUTZ_CTRL_copy_IDX, Bitlist,
		   active_EINKLEMMSCHUTZ_CTRL_IDX);
      SYS_bit_cpy (Bitlist, active_BLOCK_ERKENNUNG_CTRL_copy_IDX, Bitlist,
		   active_BLOCK_ERKENNUNG_CTRL_IDX);
      FH_TUERMODUL_CTRL__N_old = FH_TUERMODUL_CTRL__N;
      FH_TUERMODUL__I_EIN_old = FH_TUERMODUL__I_EIN;
      FH_DU__MFH = FH_DU__MFH_copy;
      FH_DU__I_EIN_old = FH_DU__I_EIN;
      BLOCK_ERKENNUNG_CTRL__N_old = BLOCK_ERKENNUNG_CTRL__N;
      FH_TUERMODUL__SFHZ_ZENTRAL_old = FH_TUERMODUL__SFHZ_ZENTRAL;
      FH_TUERMODUL__SFHZ_MEC_old = FH_TUERMODUL__SFHZ_MEC;
      FH_TUERMODUL__SFHA_ZENTRAL_old = FH_TUERMODUL__SFHA_ZENTRAL;
      FH_TUERMODUL__SFHA_MEC_old = FH_TUERMODUL__SFHA_MEC;
      FH_TUERMODUL__BLOCK = FH_TUERMODUL__BLOCK_copy;
      FH_TUERMODUL__BLOCK_old = FH_TUERMODUL__BLOCK;
      FH_TUERMODUL__SFHZ = FH_TUERMODUL__SFHZ_copy;
      FH_TUERMODUL__SFHZ_old = FH_TUERMODUL__SFHZ;
      FH_TUERMODUL__SFHA = FH_TUERMODUL__SFHA_copy;
      FH_TUERMODUL__SFHA_old = FH_TUERMODUL__SFHA;
      FH_TUERMODUL__MFHZ = FH_TUERMODUL__MFHZ_copy;
      FH_TUERMODUL__MFHZ_old = FH_TUERMODUL__MFHZ;
      FH_TUERMODUL__MFHA = FH_TUERMODUL__MFHA_copy;
      FH_TUERMODUL__MFHA_old = FH_TUERMODUL__MFHA;
      FH_TUERMODUL__EKS_LEISTE_AKTIV_old = FH_TUERMODUL__EKS_LEISTE_AKTIV;
      FH_DU__EKS_LEISTE_AKTIV_old = FH_DU__EKS_LEISTE_AKTIV;
      FH_DU__S_FH_TMBFAUFCAN_old = FH_DU__S_FH_TMBFAUFCAN;
      FH_DU__S_FH_TMBFZUCAN_old = FH_DU__S_FH_TMBFZUCAN;
      FH_DU__S_FH_TMBFZUDISC_old = FH_DU__S_FH_TMBFZUDISC;
      FH_DU__S_FH_TMBFAUFDISC_old = FH_DU__S_FH_TMBFAUFDISC;
      FH_DU__BLOCK = FH_DU__BLOCK_copy;
      FH_DU__BLOCK_old = FH_DU__BLOCK;
      FH_DU__MFHZ = FH_DU__MFHZ_copy;
      FH_DU__MFHZ_old = FH_DU__MFHZ;
      FH_DU__MFHA = FH_DU__MFHA_copy;
      FH_DU__MFHA_old = FH_DU__MFHA;

    }
    /** while(!stable) **/

}/** FH_DU **/



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
      memset (Bitlist, 0, 64 * sizeof (Bitlist[0]));
      init ();

      interface ();
      FH_DU ();
    }

  return 0;
}

void
initialise_benchmark ()
{
}

int
verify_benchmark (int unused)
{
  char expected[64] = {
    0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0
  };
  int i;

  for (i = 0; i < 64; i++)
    {
      if (Bitlist[i] != expected[i])
	return 0;
    }
  if (tm_entered_EINSCHALTSTROM_MESSEN_BLOCK_ERKENNUNG_CTRLch_BLOCK_ERKENNUNG_CTRL__N_copy != 0 || tm_entered_WIEDERHOLSPERRE_FH_TUERMODUL_CTRLexited_BEREIT_FH_TUERMODUL_CTRL != 0 || tm_entered_WIEDERHOLSPERRE_FH_TUERMODUL_CTRL != 0 || NICHT_INITIALISIERT_NICHT_INITIALISIERT_next_state != 0 || ZENTRAL_KINDERSICHERUNG_CTRL_next_state != 0 || MEC_KINDERSICHERUNG_CTRL_next_state != 0 || KINDERSICHERUNG_CTRL_KINDERSICHERUNG_CTRL_next_state != 3 || B_FH_TUERMODUL_CTRL_next_state != 2 || A_FH_TUERMODUL_CTRL_next_state != 1 || WIEDERHOLSPERRE_FH_TUERMODUL_CTRL_next_state != 1 || INITIALISIERT_FH_TUERMODUL_CTRL_next_state != 0 || TIPP_SCHLIESSEN_FH_TUERMODUL_CTRL_next_state != 0 || MANUELL_SCHLIESSEN_FH_TUERMODUL_CTRL_next_state != 0 || OEFFNEN_FH_TUERMODUL_CTRL_next_state != 0 || SCHLIESSEN_FH_TUERMODUL_CTRL_next_state != 0 || FH_STEUERUNG_DUMMY_FH_STEUERUNG_DUMMY_next_state != 2 || EINKLEMMSCHUTZ_CTRL_EINKLEMMSCHUTZ_CTRL_next_state != 1 || BEWEGUNG_BLOCK_ERKENNUNG_CTRL_next_state != 0 || BLOCK_ERKENNUNG_CTRL_BLOCK_ERKENNUNG_CTRL_next_state != 1)
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
