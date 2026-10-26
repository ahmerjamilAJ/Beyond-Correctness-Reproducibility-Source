#ifndef LLFI_PROJECT_PRIVATE_RNG_H
#define LLFI_PROJECT_PRIVATE_RNG_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

_Static_assert(sizeof(unsigned int) == 4 && sizeof(unsigned long long) == 8,
               "Qualified LLFI private RNG requires unsigned32/unsigned-long-long64 ABI");

/* Serial process-private state. The application rand()/srand() state is unused.
 * This header belongs only to the isolated generic runtime translation unit.
 * rand_r is a PRNG primitive, not a claim of physical or cryptographic entropy. */
static unsigned int llfi_private_rng_state;
static unsigned int llfi_private_rng_initial_seed;
static unsigned long long llfi_private_rng_draw_count;
static int llfi_private_rng_seeded;

static void llfi_private_seed(unsigned int seed) {
  llfi_private_rng_state = seed;
  llfi_private_rng_initial_seed = seed;
  llfi_private_rng_draw_count = 0;
  llfi_private_rng_seeded = 1;
}

static unsigned int llfi_private_raw(void) {
  if (!llfi_private_rng_seeded) {
    fprintf(stderr, "LLFI private RNG: use before entropy initialization\n");
    exit(1);
  }
  unsigned int value = (unsigned int)rand_r(&llfi_private_rng_state);
  ++llfi_private_rng_draw_count;
  return value;
}

/* Consumes exactly one private draw even at p=0/1. U is in [0,1), using
 * RAND_MAX+1 as denominator; p=0 is always false and p=1 always true.
 * Interior Bernoulli probabilities retain the finite PRNG grid. */
static int llfi_private_decision(double probability) {
  double u = llfi_private_raw() / ((double)RAND_MAX + 1.0);
  return u < probability;
}

/* Rejection, not scale-and-truncate, excludes the RAND_MAX endpoint overflow
 * and removes arithmetic modulo bias conditional on uniform primitive draws.
 * Uses the fewest radix=(RAND_MAX+1) digits needed for any unsigned bound.
 * Before extending span, span<bound<=UINT_MAX and radix<=INT_MAX+1, so the
 * product fits uint64_t for the supported 32-bit unsigned ABI. */
static unsigned int llfi_private_bounded(unsigned int bound) {
  if (bound == 0) {
    fprintf(stderr, "LLFI private RNG: zero bounded-selection width\n");
    exit(1);
  }
  const uint64_t radix = (uint64_t)RAND_MAX + 1;
  uint64_t span = radix;
  unsigned int digits = 1;
  while (span < (uint64_t)bound) {
    span *= radix;
    ++digits;
  }
  const uint64_t limit = span - span % bound;
  for (;;) {
    uint64_t value = llfi_private_raw();
    for (unsigned int digit = 1; digit < digits; ++digit)
      value = value * radix + llfi_private_raw();
    if (value < limit) return (unsigned int)(value % bound);
  }
}

#endif
