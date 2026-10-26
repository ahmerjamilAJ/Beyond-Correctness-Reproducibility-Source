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

/* BEEBS slre benchmark

   Copyright (c) 2004-2013 Sergey Lyubka <valenok@gmail.com>
   Copyright (c) 2013 Cesanta Software Limited.  All rights reserved
   Copyright (C) 2014 Embecosm Limited and University of Bristol

   This version, copyright (C) 2014-2019 Embecosm Limited and University of
   Bristol

   Contributor James Pallister <james.pallister@bristol.ac.uk>
   Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>

   This file is part of Embench and was formerly part of the Bristol/Embecosm
   Embedded Benchmark Suite.

   SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef SLRE_HEADER_DEFINED
#define SLRE_HEADER_DEFINED

#ifdef __cplusplus
extern "C"
{
#endif

  struct slre_cap
  {
    const char *ptr;
    int len;
  };

  int slre_match (const char *regexp, const char *buf, int buf_len,
		  struct slre_cap *caps, int num_caps);

/* slre_match() failure codes */
#define SLRE_NO_MATCH               -1
#define SLRE_UNEXPECTED_QUANTIFIER  -2
#define SLRE_UNBALANCED_BRACKETS    -3
#define SLRE_INTERNAL_ERROR         -4
#define SLRE_INVALID_CHARACTER_SET  -5
#define SLRE_INVALID_METACHARACTER  -6
#define SLRE_CAPS_ARRAY_TOO_SMALL   -7
#define SLRE_TOO_MANY_BRANCHES      -8
#define SLRE_TOO_MANY_BRACKETS      -9

#ifdef __cplusplus
}
#endif

#endif				/* SLRE_HEADER_DEFINED */


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

/* BEEBS slre benchmark

   Copyright (c) 2004-2013 Sergey Lyubka <valenok@gmail.com>
   Copyright (c) 2013 Cesanta Software Limited All rights reserved

   This version, copyright (C) 2014-2019 Embecosm Limited and University of
   Bristol

   Contributor James Pallister <james.pallister@bristol.ac.uk>
   Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>

   This file is part of Embench and was formerly part of the Bristol/Embecosm
   Embedded Benchmark Suite.

   SPDX-License-Identifier: GPL-3.0-or-later */



/* This scale factor will be changed to equalise the runtime of the
   benchmarks. */
#define LOCAL_SCALE_FACTOR 110

#include <stdio.h>
#include <ctype.h>
#include <string.h>



#define MAX_BRANCHES 100
#define MAX_BRACKETS 100
#define ARRAY_SIZE(ar) (int) (sizeof(ar) / sizeof((ar)[0]))
#define FAIL_IF(condition, error_code) if (condition) return (error_code)

#ifdef SLRE_DEBUG
#define DBG(x) printf x
#else
#define DBG(x)
#endif

struct bracket_pair
{
  const char *ptr;		/* Points to the first char after '(' in regex  */
  int len;			/* Length of the text between '(' and ')'       */
  int branches;			/* Index in the branches array for this pair    */
  int num_branches;		/* Number of '|' in this bracket pair           */
};

struct branch
{
  int bracket_index;		/* index for 'struct bracket_pair brackets' */
  /* array defined below                      */
  const char *schlong;		/* points to the '|' character in the regex */
};

struct regex_info
{
  /*
   * Describes all bracket pairs in the regular expression.
   * First entry is always present, and grabs the whole regex.
   */
  struct bracket_pair brackets[MAX_BRACKETS];
  int num_brackets;

  /*
   * Describes alternations ('|' operators) in the regular expression.
   * Each branch falls into a specific branch pair.
   */
  struct branch branches[MAX_BRANCHES];
  int num_branches;

  /* Array of captures provided by the user */
  struct slre_cap *caps;
  int num_caps;

  /* E.g. IGNORE_CASE. See enum below */
  int flags;
};
enum
{ IGNORE_CASE = 1 };

static int
is_metacharacter (const unsigned char *s)
{
  static const char *metacharacters = "^$().[]*+?|\\Ssd";
  return strchr (metacharacters, *s) != NULL;
}

static int
op_len (const char *re)
{
  return re[0] == '\\' && re[1] == 'x' ? 4 : re[0] == '\\' ? 2 : 1;
}

static int
set_len (const char *re, int re_len)
{
  int len = 0;

  while (len < re_len && re[len] != ']')
    {
      len += op_len (re + len);
    }

  return len <= re_len ? len + 1 : -1;
}

static int
get_op_len (const char *re, int re_len)
{
  return re[0] == '[' ? set_len (re + 1, re_len - 1) + 1 : op_len (re);
}

static int
is_quantifier (const char *re)
{
  return re[0] == '*' || re[0] == '+' || re[0] == '?';
}

static int
toi (int x)
{
  return isdigit (x) ? x - '0' : x - 'W';
}

static int
hextoi (const unsigned char *s)
{
  return (toi (tolower (s[0])) << 4) | toi (tolower (s[1]));
}

static int
match_op (const unsigned char *re, const unsigned char *s,
	  struct regex_info *info)
{
  int result = 0;
  switch (*re)
    {
    case '\\':
      /* Metacharacters */
      switch (re[1])
	{
	case 'S':
	  FAIL_IF (isspace (*s), SLRE_NO_MATCH);
	  result++;
	  break;

	case 's':
	  FAIL_IF (!isspace (*s), SLRE_NO_MATCH);
	  result++;
	  break;

	case 'd':
	  FAIL_IF (!isdigit (*s), SLRE_NO_MATCH);
	  result++;
	  break;

	case 'x':
	  /* Match byte, \xHH where HH is hexadecimal byte representaion */
	  FAIL_IF (hextoi (re + 2) != *s, SLRE_NO_MATCH);
	  result++;
	  break;

	default:
	  /* Valid metacharacter check is done in bar() */
	  FAIL_IF (re[1] != s[0], SLRE_NO_MATCH);
	  result++;
	  break;
	}
      break;

    case '|':
      FAIL_IF (1, SLRE_INTERNAL_ERROR);
      break;
    case '$':
      FAIL_IF (1, SLRE_NO_MATCH);
      break;
    case '.':
      result++;
      break;

    default:
      if (info->flags & IGNORE_CASE)
	{
	  FAIL_IF (tolower (*re) != tolower (*s), SLRE_NO_MATCH);
	}
      else
	{
	  FAIL_IF (*re != *s, SLRE_NO_MATCH);
	}
      result++;
      break;
    }

  return result;
}

static int
match_set (const char *re, int re_len, const char *s, struct regex_info *info)
{
  int len = 0, result = -1, invert = re[0] == '^';

  if (invert)
    re++, re_len--;

  while (len <= re_len && re[len] != ']' && result <= 0)
    {
      /* Support character range */
      if (re[len] != '-' && re[len + 1] == '-' && re[len + 2] != ']' &&
	  re[len + 2] != '\0')
	{
	  result = info->flags && IGNORE_CASE ?
	    *s >= re[len] && *s <= re[len + 2] :
	    tolower ((int) *s) >= tolower ((int) re[len])
	    && tolower ((int) *s) <= tolower ((int) re[len + 2]);
	  len += 3;
	}
      else
	{
	  result =
	    match_op ((unsigned char *) re + len, (unsigned char *) s, info);
	  len += op_len (re + len);
	}
    }
  return (!invert && result > 0) || (invert && result <= 0) ? 1 : -1;
}

static int doh (const char *s, int s_len, struct regex_info *info, int bi);

static int
bar (const char *re, int re_len, const char *s, int s_len,
     struct regex_info *info, int bi)
{
  /* i is offset in re, j is offset in s, bi is brackets index */
  int i, j, n, step;

  for (i = j = 0; i < re_len && j <= s_len; i += step)
    {

      /* Handle quantifiers. Get the length of the chunk. */
      step = re[i] == '(' ? info->brackets[bi + 1].len + 2 :
	get_op_len (re + i, re_len - i);

      DBG (("%s [%.*s] [%.*s] re_len=%d step=%d i=%d j=%d\n", __func__,
	    re_len - i, re + i, s_len - j, s + j, re_len, step, i, j));

      FAIL_IF (is_quantifier (&re[i]), SLRE_UNEXPECTED_QUANTIFIER);
      FAIL_IF (step <= 0, SLRE_INVALID_CHARACTER_SET);

      if (i + step < re_len && is_quantifier (re + i + step))
	{
	  DBG (("QUANTIFIER: [%.*s]%c [%.*s]\n", step, re + i,
		re[i + step], s_len - j, s + j));
	  if (re[i + step] == '?')
	    {
	      int result = bar (re + i, step, s + j, s_len - j, info, bi);
	      j += result > 0 ? result : 0;
	      i++;
	    }
	  else if (re[i + step] == '+' || re[i + step] == '*')
	    {
	      int j2 = j, nj = j, n1, n2 = -1, ni, non_greedy = 0;

	      /* Points to the regexp code after the quantifier */
	      ni = i + step + 1;
	      if (ni < re_len && re[ni] == '?')
		{
		  non_greedy = 1;
		  ni++;
		}

	      do
		{
		  if ((n1 =
		       bar (re + i, step, s + j2, s_len - j2, info, bi)) > 0)
		    {
		      j2 += n1;
		    }
		  if (re[i + step] == '+' && n1 < 0)
		    break;

		  if (ni >= re_len)
		    {
		      /* After quantifier, there is nothing */
		      nj = j2;
		    }
		  else if ((n2 = bar (re + ni, re_len - ni, s + j2,
				      s_len - j2, info, bi)) >= 0)
		    {
		      /* Regex after quantifier matched */
		      nj = j2 + n2;
		    }
		  if (nj > j && non_greedy)
		    break;
		}
	      while (n1 > 0);

	      if (n1 < 0 && re[i + step] == '*' &&
		  (n2 =
		   bar (re + ni, re_len - ni, s + j, s_len - j, info,
			bi)) > 0)
		{
		  nj = j + n2;
		}

	      DBG (("STAR/PLUS END: %d %d %d %d %d\n", j, nj, re_len - ni, n1,
		    n2));
	      FAIL_IF (re[i + step] == '+' && nj == j, SLRE_NO_MATCH);

	      /* If while loop body above was not executed for the * quantifier,  */
	      /* make sure the rest of the regex matches                          */
	      FAIL_IF (nj == j && ni < re_len && n2 < 0, SLRE_NO_MATCH);

	      /* Returning here cause we've matched the rest of RE already */
	      return nj;
	    }
	  continue;
	}

      if (re[i] == '[')
	{
	  n = match_set (re + i + 1, re_len - (i + 2), s + j, info);
	  DBG (("SET %.*s [%.*s] -> %d\n", step, re + i, s_len - j, s + j,
		n));
	  FAIL_IF (n <= 0, SLRE_NO_MATCH);
	  j += n;
	}
      else if (re[i] == '(')
	{
	  n = SLRE_NO_MATCH;
	  bi++;
	  FAIL_IF (bi >= info->num_brackets, SLRE_INTERNAL_ERROR);
	  DBG (("CAPTURING [%.*s] [%.*s] [%s]\n",
		step, re + i, s_len - j, s + j, re + i + step));

	  if (re_len - (i + step) <= 0)
	    {
	      /* Nothing follows brackets */
	      n = doh (s + j, s_len - j, info, bi);
	    }
	  else
	    {
	      int j2;
	      for (j2 = 0; j2 <= s_len - j; j2++)
		{
		  if ((n = doh (s + j, s_len - (j + j2), info, bi)) >= 0 &&
		      bar (re + i + step, re_len - (i + step),
			   s + j + n, s_len - (j + n), info, bi) >= 0)
		    break;
		}
	    }

	  DBG (("CAPTURED [%.*s] [%.*s]:%d\n", step, re + i, s_len - j, s + j,
		n));
	  FAIL_IF (n < 0, n);
	  if (info->caps != NULL)
	    {
	      info->caps[bi - 1].ptr = s + j;
	      info->caps[bi - 1].len = n;
	    }
	  j += n;
	}
      else if (re[i] == '^')
	{
	  FAIL_IF (j != 0, SLRE_NO_MATCH);
	}
      else if (re[i] == '$')
	{
	  FAIL_IF (j != s_len, SLRE_NO_MATCH);
	}
      else
	{
	  FAIL_IF (j >= s_len, SLRE_NO_MATCH);
	  n =
	    match_op ((unsigned char *) (re + i), (unsigned char *) (s + j),
		      info);
	  FAIL_IF (n <= 0, n);
	  j += n;
	}
    }

  return j;
}

/* Process branch points */
static int
doh (const char *s, int s_len, struct regex_info *info, int bi)
{
  const struct bracket_pair *b = &info->brackets[bi];
  int i = 0, len, result;
  const char *p;

  do
    {
      p = i == 0 ? b->ptr : info->branches[b->branches + i - 1].schlong + 1;
      len = b->num_branches == 0 ? b->len :
	i == b->num_branches ? b->ptr + b->len - p :
	info->branches[b->branches + i].schlong - p;
      DBG (("%s %d %d [%.*s] [%.*s]\n", __func__, bi, i, len, p, s_len, s));
      result = bar (p, len, s, s_len, info, bi);
      DBG (("%s <- %d\n", __func__, result));
    }
  while (result <= 0 && i++ < b->num_branches);	/* At least 1 iteration */

  return result;
}

static int
baz (const char *s, int s_len, struct regex_info *info)
{
  int i, result = -1, is_anchored = info->brackets[0].ptr[0] == '^';

  for (i = 0; i <= s_len; i++)
    {
      result = doh (s + i, s_len - i, info, 0);
      if (result >= 0)
	{
	  result += i;
	  break;
	}
      if (is_anchored)
	break;
    }

  return result;
}

static void
setup_branch_points (struct regex_info *info)
{
  int i, j;
  struct branch tmp;

  /* First, sort branches. Must be stable, no qsort. Use bubble algo. */
  for (i = 0; i < info->num_branches; i++)
    {
      for (j = i + 1; j < info->num_branches; j++)
	{
	  if (info->branches[i].bracket_index >
	      info->branches[j].bracket_index)
	    {
	      tmp = info->branches[i];
	      info->branches[i] = info->branches[j];
	      info->branches[j] = tmp;
	    }
	}
    }

  /*
   * For each bracket, set their branch points. This way, for every bracket
   * (i.e. every chunk of regex) we know all branch points before matching.
   */
  for (i = j = 0; i < info->num_brackets; i++)
    {
      info->brackets[i].num_branches = 0;
      info->brackets[i].branches = j;
      while (j < info->num_branches && info->branches[j].bracket_index == i)
	{
	  info->brackets[i].num_branches++;
	  j++;
	}
    }
}

static int
foo (const char *re, int re_len, const char *s, int s_len,
     struct regex_info *info)
{
  int i, step, depth = 0;

  /* First bracket captures everything */
  info->brackets[0].ptr = re;
  info->brackets[0].len = re_len;
  info->num_brackets = 1;

  /* Make a single pass over regex string, memorize brackets and branches */
  for (i = 0; i < re_len; i += step)
    {
      step = get_op_len (re + i, re_len - i);

      if (re[i] == '|')
	{
	  FAIL_IF (info->num_branches >= ARRAY_SIZE (info->branches),
		   SLRE_TOO_MANY_BRANCHES);
	  info->branches[info->num_branches].bracket_index =
	    info->brackets[info->num_brackets - 1].len == -1 ?
	    info->num_brackets - 1 : depth;
	  info->branches[info->num_branches].schlong = &re[i];
	  info->num_branches++;
	}
      else if (re[i] == '\\')
	{
	  FAIL_IF (i >= re_len - 1, SLRE_INVALID_METACHARACTER);
	  if (re[i + 1] == 'x')
	    {
	      /* Hex digit specification must follow */
	      FAIL_IF (re[i + 1] == 'x' && i >= re_len - 3,
		       SLRE_INVALID_METACHARACTER);
	      FAIL_IF (re[i + 1] == 'x'
		       && !(isxdigit ((unsigned char) re[i + 2])
			    && isxdigit ((unsigned char) re[i + 3])),
		       SLRE_INVALID_METACHARACTER);
	    }
	  else
	    {
	      FAIL_IF (!is_metacharacter ((unsigned char *) re + i + 1),
		       SLRE_INVALID_METACHARACTER);
	    }
	}
      else if (re[i] == '(')
	{
	  FAIL_IF (info->num_brackets >= ARRAY_SIZE (info->brackets),
		   SLRE_TOO_MANY_BRACKETS);
	  depth++;		/* Order is important here. Depth increments first. */
	  info->brackets[info->num_brackets].ptr = re + i + 1;
	  info->brackets[info->num_brackets].len = -1;
	  info->num_brackets++;
	  FAIL_IF (info->num_caps > 0
		   && info->num_brackets - 1 > info->num_caps,
		   SLRE_CAPS_ARRAY_TOO_SMALL);
	}
      else if (re[i] == ')')
	{
	  int ind = info->brackets[info->num_brackets - 1].len == -1 ?
	    info->num_brackets - 1 : depth;
	  info->brackets[ind].len = &re[i] - info->brackets[ind].ptr;
	  DBG (("SETTING BRACKET %d [%.*s]\n",
		ind, info->brackets[ind].len, info->brackets[ind].ptr));
	  depth--;
	  FAIL_IF (depth < 0, SLRE_UNBALANCED_BRACKETS);
	  FAIL_IF (i > 0 && re[i - 1] == '(', SLRE_NO_MATCH);
	}
    }

  FAIL_IF (depth != 0, SLRE_UNBALANCED_BRACKETS);
  setup_branch_points (info);

  return baz (s, s_len, info);
}

int
slre_match (const char *regexp, const char *s, int s_len,
	    struct slre_cap *caps, int num_caps)
{
  struct regex_info info;

  /* Initialize info structure */
  info.flags = info.num_brackets = info.num_branches = 0;
  info.num_caps = num_caps;
  info.caps = caps;

  DBG (("========================> [%s] [%.*s]\n", regexp, s_len, s));

  /* Handle regexp flags. At the moment, only 'i' is supported */
  if (memcmp (regexp, "(?i)", 4) == 0)
    {
      info.flags |= IGNORE_CASE;
      regexp += 4;
    }

  return foo (regexp, strlen (regexp), s, s_len, &info);
}

char text[] = "abbbababaabccababcacbcbcbabbabcbabcabcbbcbbac";
char *regexes[] = { "(ab)+", "(b.+)+", "a[ab]*", "([ab^c][ab^c])+" };

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
  volatile int ret;
  int j;

  for (j = 0; j < rpt; j++)
    {
      int i;
      int len = strlen (text);
      struct slre_cap captures;
      ret = 0;

      for (i = 0; i < 4; ++i)
	{
	  ret += slre_match (regexes[i], text, len, &captures, 1);
	}
    }

  return ret;
}


int
verify_benchmark (int r)
{
  return 102 == r;
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
