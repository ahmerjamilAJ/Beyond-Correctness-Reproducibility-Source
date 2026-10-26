/* Beyond Correctness source packaging notice, 2026-10-25.
 * This is the research-adapted/amalgamated benchmark version.
 * Its upstream copyright and license notices are retained below.
 * This release adds only this notice and a line-number directive.
 * See ../../licenses/ and the benchmark catalog for notices.
 */
#line 1
/***************************************************************************
 *cr
 *cr            (C) Copyright 2007 The Board of Trustees of the
 *cr                        University of Illinois
 *cr                         All Rights Reserved
 *cr
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <inttypes.h>
/***************************************************************************
 *cr
 *cr            (C) Copyright 2007 The Board of Trustees of the
 *cr                        University of Illinois
 *cr                         All Rights Reserved
 *cr
 ***************************************************************************/

/* Search offsets within 16 pixels of (0,0) */
#define SEARCH_RANGE 16

/* The total search area is 33 pixels square */
#define SEARCH_DIMENSION (2*SEARCH_RANGE+1)

/* The total number of search positions is 33^2 */
#define MAX_POS 1089

/* This is padded to a multiple of 4 when allocating memory */
#define MAX_POS_PADDED 1092

/* VBSME block indices in the SAD array for different 
 * block sizes.  The index is computed from the
 * image size in macroblocks.  Block sizes are (height, width):
 *  1: 16 by 16 pixels, one block per macroblock
 *  2: 8  by 16 pixels, 2  blocks per macroblock
 *  3: 16 by 8  pixels, 2  blocks per macroblock
 *  4: 8  by 8  pixels, 4  blocks per macroblock
 *  5: 4  by 8  pixels, 8  blocks per macroblock
 *  6: 8  by 4  pixels, 8  blocks per macroblock
 *  7: 4  by 4  pixels, 16 blocks per macroblock
 */
#define SAD_TYPE_1_IX(image_size) 0
#define SAD_TYPE_2_IX(image_size) ((image_size)*MAX_POS_PADDED)
#define SAD_TYPE_3_IX(image_size) ((image_size)*(3*MAX_POS_PADDED))
#define SAD_TYPE_4_IX(image_size) ((image_size)*(5*MAX_POS_PADDED))
#define SAD_TYPE_5_IX(image_size) ((image_size)*(9*MAX_POS_PADDED))
#define SAD_TYPE_6_IX(image_size) ((image_size)*(17*MAX_POS_PADDED))
#define SAD_TYPE_7_IX(image_size) ((image_size)*(25*MAX_POS_PADDED))

#define SAD_TYPE_IX(n, image_size) \
  ((n == 1) ? SAD_TYPE_1_IX(image_size) : \
   ((n == 2) ? SAD_TYPE_2_IX(image_size) : \
    ((n == 3) ? SAD_TYPE_3_IX(image_size) : \
     ((n == 4) ? SAD_TYPE_4_IX(image_size) : \
      ((n == 5) ? SAD_TYPE_5_IX(image_size) : \
       ((n == 6) ? SAD_TYPE_6_IX(image_size) : \
        (SAD_TYPE_7_IX(image_size) \
	 )))))))

#define SAD_TYPE_1_CT 1
#define SAD_TYPE_2_CT 2
#define SAD_TYPE_3_CT 2
#define SAD_TYPE_4_CT 4
#define SAD_TYPE_5_CT 8
#define SAD_TYPE_6_CT 8
#define SAD_TYPE_7_CT 16

#define SAD_TYPE_CT(n) \
  ((n == 1) ? SAD_TYPE_1_CT : \
   ((n == 2) ? SAD_TYPE_2_CT : \
    ((n == 3) ? SAD_TYPE_3_CT : \
     ((n == 4) ? SAD_TYPE_4_CT : \
      ((n == 5) ? SAD_TYPE_5_CT : \
       ((n == 6) ? SAD_TYPE_6_CT : \
        (SAD_TYPE_7_CT \
	 )))))))

#ifdef __cplusplus
extern "C" {
#endif

void sad4_cpu(unsigned short *blk_sad,
	      unsigned short *frame,
	      unsigned short *ref,
	      int mb_width,
	      int mb_height);

void larger_sads(unsigned short *sads,
		 int mbs);

#ifdef __cplusplus
}
#endif

/***************************************************************************
 *cr
 *cr            (C) Copyright 2007 The Board of Trustees of the
 *cr                        University of Illinois
 *cr                         All Rights Reserved
 *cr
 ***************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned short read16u(FILE *f);
short read16i(FILE *f);

void write32u(FILE *f, unsigned int i);
void write16u(FILE *f, unsigned short h);
void write16i(FILE *f, short h);

#ifdef __cplusplus
}
#endif

/***************************************************************************
 *cr
 *cr            (C) Copyright 2007 The Board of Trustees of the
 *cr                        University of Illinois
 *cr                         All Rights Reserved
 *cr
 ***************************************************************************/

struct image_i16
{
  int width;
  int height;
  short *data;
};

#ifdef __cplusplus
extern "C" {
#endif

struct image_i16 * load_image(char *filename);
void free_image(struct image_i16 *);

#ifdef __cplusplus
}
#endif


static unsigned short *
load_sads(char *filename);
static void
write_sads(char *filename,
	   int image_width_macroblocks,
	   int image_height_macroblocks,
	   unsigned short *sads);
static void
write_sads_directly(char *filename,
		    int width,
		    int height,
		    unsigned short *sads);

/* FILE I/O */

unsigned short *
load_sads(char *filename)
{
  FILE *infile;
  unsigned short *sads;
  int w;
  int h;
  int sads_per_block;

  infile = fopen(filename, "r");

  if (!infile)
    {
      fprintf(stderr, "Cannot find file '%s'\n", filename);
      exit(-1);
    }

  /* Read image dimensions (measured in macroblocks) */
  w = read16u(infile);
  h = read16u(infile);

  /* Read SAD values.  Only interested in the 4x4 SAD values, which are
   * at the end of the file. */
  sads_per_block = MAX_POS_PADDED * (w * h);
  fseek(infile, 25 * sads_per_block * sizeof(unsigned short), SEEK_CUR);

  sads = (unsigned short *)malloc(sads_per_block * 16 * sizeof(unsigned short));
  fread(sads, sizeof(unsigned short), sads_per_block * 16, infile);
  fclose(infile);

  return sads;
}

/* Compare the reference SADs to the expected SADs.
 */
void
check_sads(unsigned short *sads_reference,
	   unsigned short *sads_computed,
	   int image_size_macroblocks)
{
  int block;

  /* Check the 4x4 SAD values.  These are in sads_reference.
   * Ignore the data at the beginning of sads_computed. */
  sads_computed += 25 * MAX_POS_PADDED * image_size_macroblocks;

  for (block = 0; block < image_size_macroblocks; block++)
    {
      int subblock;

      for (subblock = 0; subblock < 16; subblock++)
	{
	  int sad_index;

	  for (sad_index = 0; sad_index < MAX_POS; sad_index++)
	    {
	      int index =
		(block * 16 + subblock) * MAX_POS_PADDED + sad_index;

	      if (sads_reference[index] != sads_computed[index])
		{
#if 0
		  /* Print exactly where the mismatch was seen */
		  printf("M %3d %2d %4d (%d = %d)\n", block, subblock, sad_index, sads_reference[index], sads_computed[index]);
#else
		  goto mismatch;
#endif
		}
	    }
	}
    }

  printf("Success.\n");
  return;

 mismatch:
  printf("Computed SADs do not match expected values.\n");
}

/* Extract the SAD data for a particular block type for a particular
 * macroblock from the array of SADs of that block type. */
static inline void
write_subblocks(FILE *outfile, unsigned short *subblock_array, int macroblock,
		int count)
{
  int block;
  int pos;

  for (block = 0; block < count; block++)
    {
      unsigned short *vec = subblock_array +
	(block + macroblock * count) * MAX_POS_PADDED;

      /* Write all SADs for this sub-block */
      for (pos = 0; pos < MAX_POS; pos++)
	write16u(outfile, *vec++);
    }
}

/* Write some SAD data to a file for output checking.
 *
 * All SAD values for six rows of macroblocks are written.
 * The six rows consist of the top two, middle two, and bottom two image rows.
 */
void
write_sads(char *filename,
	   int mb_width,
	   int mb_height,
	   unsigned short *sads)
{
  FILE *outfile = fopen(filename, "w");
  int mbs = mb_width * mb_height;
  int row_indir;
  int row_indices[6] = {0, 1,
			mb_height / 2 - 1, mb_height / 2,
			mb_height - 2, mb_height - 1};

  if (outfile == NULL)
    {
      fprintf(stderr, "Cannot open output file\n");
      exit(-1);
    }

  /* Write the number of output macroblocks */
  write32u(outfile, mb_width * 6);

  /* Write zeros */
  write32u(outfile, 0);

  /* Each row */
  for (row_indir = 0; row_indir < 6; row_indir++)
    {
      int row = row_indices[row_indir];

      /* Each block in row */
      int block;
      for (block = mb_width * row; block < mb_width * (row + 1); block++)
	{
	  int blocktype;

	  /* Write SADs for all sub-block types */
	  for (blocktype = 1; blocktype <= 7; blocktype++)
	    write_subblocks(outfile,
			    sads + SAD_TYPE_IX(blocktype, mbs),
			    block,
			    SAD_TYPE_CT(blocktype));
	}
    }

  fclose(outfile);
}

/* FILE I/O for debugging */

static void
write_sads_directly(char *filename,
		    int width,
		    int height,
		    unsigned short *sads)
{
  FILE *f = fopen(filename, "w");
  int n;

  write16u(f, width);
  write16u(f, height);
  for (n = 0; n < 41 * MAX_POS_PADDED * (width * height); n++) {
    write16u(f, sads[n]);
  }
  fclose(f);
}

static void
print_test_sad_vector(unsigned short *base, int macroblock, int count)
{
  int n;
  int searchpos = 17*33+17;
  for (n = 0; n < count; n++)
    printf(" %d", base[(count * macroblock + n) * MAX_POS_PADDED + searchpos]);
}

static void
print_test_sads(unsigned short *sads_computed,
		int mbs)
{
  int macroblock = 5;
  int blocktype;

  for (blocktype = 1; blocktype <= 7; blocktype++)
    {
      printf("%d:", blocktype);
      print_test_sad_vector(sads_computed + SAD_TYPE_IX(blocktype, mbs),
			    macroblock, SAD_TYPE_CT(blocktype));
      puts("\n");
    }
}

/* MAIN */

int
main(int argc, char **argv)
{
  struct image_i16 *ref_image;
  struct image_i16 *cur_image;
  unsigned short *sads_computed; /* SADs generated by the program */

  int image_size_bytes;
  int image_size_macroblocks;
  int image_width_macroblocks;
  int image_height_macroblocks;




  if (argc != 3) return 2;
  /* Read input files */
  ref_image = load_image(argv[1]);
  cur_image = load_image(argv[2]);

  if ((ref_image->width != cur_image->width) ||
      (ref_image->height != cur_image->height))
    {
      fprintf(stderr, "Input images must be the same size\n");
      exit(-1);
    }
  if ((ref_image->width % 16) || (ref_image->height % 16))
    {
      fprintf(stderr, "Input image size must be an integral multiple of 16\n");
      exit(-1);
    }

  /* Compute parameters, allocate memory */
  image_size_bytes = ref_image->width * ref_image->height * sizeof(short);
  image_width_macroblocks = ref_image->width / 16;
  image_height_macroblocks = ref_image->height / 16;
  image_size_macroblocks = image_width_macroblocks * image_height_macroblocks;
  
  sads_computed = (unsigned short *)
    malloc(41 * MAX_POS_PADDED * image_size_macroblocks * sizeof(short));

  /* Run the kernel code */
  sad4_cpu(sads_computed,
	   (unsigned short *)cur_image->data,
	   (unsigned short *)ref_image->data,
	   ref_image->width / 16, ref_image->height / 16);
  larger_sads(sads_computed, image_size_macroblocks);

  /* Print output */
  write_sads((char*)"sad.bin", image_width_macroblocks, image_height_macroblocks, sads_computed);

#if 0  /* Debugging */
  print_test_sads(sads_computed, image_size_macroblocks);
  write_sads_directly("sad-debug.bin",
		      ref_image->width / 16, ref_image->height / 16,
		      sads_computed);
#endif

  /* Free memory */
  free(sads_computed);
  free_image(ref_image);
  free_image(cur_image);



  return 0;
}

/***************************************************************************
 *cr
 *cr            (C) Copyright 2007 The Board of Trustees of the
 *cr                        University of Illinois
 *cr                         All Rights Reserved
 *cr
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>



struct image_i16 *
load_image(char *filename)
{
  FILE *infile;
  short *data;
  int w;
  int h;

  infile = fopen(filename, "r");

  if (!infile)
    {
      fprintf(stderr, "Cannot find file '%s'\n", filename);
      exit(-1);
    }

  /* Read image dimensions */
  w = read16u(infile);
  h = read16u(infile);

  /* Read image contents */
  data = (short *)malloc(w * h * sizeof(short));
  fread(data, sizeof(short), w * h, infile);

  fclose(infile);

  /* Create the return data structure */
  {
    struct image_i16 *ret =
      (struct image_i16 *)malloc(sizeof(struct image_i16));
    ret->width = w;
    ret->height = h;
    ret->data = data;
    return ret;
  }
}

void
free_image(struct image_i16 *img)
{
  free(img->data);
  free(img);
}

/***************************************************************************
 *cr
 *cr            (C) Copyright 2007 The Board of Trustees of the
 *cr                        University of Illinois
 *cr                         All Rights Reserved
 *cr
 ***************************************************************************/

#include <stdio.h>


unsigned short
read16u(FILE *f)
{
  int n;

  n = fgetc(f);
  n += fgetc(f) << 8;

  return n;
}

short
read16i(FILE *f)
{
  int n;

  n = fgetc(f);
  n += fgetc(f) << 8;

  return n;
}

void
write32u(FILE *f, unsigned int i)
{
  putc(i, f);
  putc(i >> 8, f);
  putc(i >> 16, f);
  putc(i >> 24, f);
}

void
write16u(FILE *f, unsigned short h)
{
  putc(h, f);
  putc(h >> 8, f);
}

void
write16i(FILE *f, short h)
{
  putc(h, f);
  putc(h >> 8, f);
}

/***************************************************************************
 *cr
 *cr            (C) Copyright 2007 The Board of Trustees of the
 *cr                        University of Illinois
 *cr                         All Rights Reserved
 *cr
 ***************************************************************************/

/* The optimized CPU-only version of the code, for performance measurement. */

#include <stdlib.h>


static short line[16];

static void sad4_one_macroblock(unsigned short *blk_sad,
				unsigned short *frame,
				unsigned short *ref,
				int org_y,
				int org_x,
				int mb_width,
				int mb_height);

static short *
create_padded_row (short *ref, int y, int x, int height, int width);

void sad4_cpu(unsigned short *blk_sad,
	      unsigned short *frame,
	      unsigned short *ref,
	      int mb_width,
	      int mb_height)
{
  int mb_x, mb_y, bk_x, bk_y;
  unsigned int frame_yoff;

  /* Go to the starting offset in blk_sad */
  blk_sad += SAD_TYPE_7_IX(mb_width * mb_height);

  /* For each block */
  for (mb_y = 0, frame_yoff = 0;
       mb_y < mb_height;
       mb_y++, frame_yoff += 256 * mb_width)
    {
      for (mb_x = 0; mb_x < mb_width; mb_x++)
	{
	  sad4_one_macroblock
	    (blk_sad + (mb_y * mb_width + mb_x) * (SAD_TYPE_7_CT * MAX_POS_PADDED),
	     frame + frame_yoff + mb_x * 16,
	     ref,
	     mb_y * 16,
	     mb_x * 16,
	     mb_width,
	     mb_height);
	}
    }
}

void
sad4_one_macroblock(unsigned short *macroblock_sad,
		    unsigned short *frame,
		    unsigned short *ref,
		    int frame_y,
		    int frame_x,
		    int mb_width,
		    int mb_height)
{
  unsigned short frame_mb[256];	/* current macroblock in frame */
  int pos_x, pos_y;
  int blky, pixy;
  int width = mb_width * 16;
  int max_width = width - 17;
  int height = mb_height * 16;
  int max_height = height - 17;
  int range_partly_outside;
  int pos;			/* search position */

  /* Make a local copy of frame */
  {
    int x, y;
    for (y = 0; y < 16; y++)
      for (x = 0; x < 16; x++)
	frame_mb[16*y+x] = frame[width * y + x];
  }

  if ((frame_x >= SEARCH_RANGE) && (frame_x <= width - SEARCH_RANGE - 17) &&
      (frame_y >= SEARCH_RANGE) && (frame_y <= height - SEARCH_RANGE - 17))
    range_partly_outside = 0;
  else
    range_partly_outside = 1;

  /* Each search position */
  pos = 0;
  for (pos_y = -SEARCH_RANGE; pos_y <= SEARCH_RANGE; pos_y++) {
    for (pos_x = -SEARCH_RANGE; pos_x <= SEARCH_RANGE; pos_x++, pos++) {
      int y;
      int abs_x, abs_y;
      int do_bounds_check;
      short *curptr = frame_mb;
      short *refptr;
      unsigned short *sad_line = macroblock_sad;

      abs_y = frame_y + pos_y;
      abs_x = frame_x + pos_x;

      do_bounds_check = range_partly_outside &&
	!(abs_y >= 0 && abs_y <= max_height &&
	  abs_x >= 0 && abs_x <= max_width);

      for (blky = 0; blky < 4; blky++)
	{
	  int sad0 = 0, sad1 = 0, sad2 = 0, sad3 = 0;

	  for (y = 0; y < 4; y++)
	    {
	      refptr = do_bounds_check
		? create_padded_row(ref, abs_y, abs_x, height, width)
		: (short *)(ref + abs_y * width + abs_x);

	      abs_y++;

	      sad0 += abs (*refptr++ - *curptr++);
	      sad0 += abs (*refptr++ - *curptr++);
	      sad0 += abs (*refptr++ - *curptr++);
	      sad0 += abs (*refptr++ - *curptr++);
	      sad1 += abs (*refptr++ - *curptr++);
	      sad1 += abs (*refptr++ - *curptr++);
	      sad1 += abs (*refptr++ - *curptr++);
	      sad1 += abs (*refptr++ - *curptr++);
	      sad2 += abs (*refptr++ - *curptr++);
	      sad2 += abs (*refptr++ - *curptr++);
	      sad2 += abs (*refptr++ - *curptr++);
	      sad2 += abs (*refptr++ - *curptr++);
	      sad3 += abs (*refptr++ - *curptr++);
	      sad3 += abs (*refptr++ - *curptr++);
	      sad3 += abs (*refptr++ - *curptr++);
	      sad3 += abs (*refptr++ - *curptr++);
	    }

	  sad_line[pos] = sad0;
	  sad_line[MAX_POS_PADDED + pos] = sad1;
	  sad_line[MAX_POS_PADDED*2 + pos] = sad2;
	  sad_line[MAX_POS_PADDED*3 + pos] = sad3;

	  sad_line += MAX_POS_PADDED*4;
	}
    }
  }
}

/* Return a row of 16 pixels starting at offset (x, y).  The row may lie
 * partly outside the image, in which case an appropriate row will be
 * constructed in 'line' and returned.  Otherwise, a reference to the
 * image is returned. */
static short *
create_padded_row (short *ref, int y, int x, int height, int width)
{
  int i, maxx;
  short *row;

  if (y < 0) y = 0;
  else if (y >= height) y = height - 1;

  row = ref + y * width;

  if ((x >= 0) && (x <= width - 16)) return row + x;

  i = 0;
  /* Pad left edge of image */
  for (; (x < 0) && (i < 16); x++, i++)
    line[i] = row[0];

  /* Copy row from image */
  for (; (x < width) && (i < 16); x++, i++)
    line[i] = row[x];

  /* Pad right edge of image */
  for (; i < 16; x++, i++)
    line[i] = row[width - 1];

  return line;
}

void larger_sads(unsigned short *sads, int mbs)
{
  int macroblock;
  int block_x, block_y;
  unsigned short *x, *y;	/* inputs to vector addition */
  unsigned short *z;		/* output of vector addition */
  int count;

#if 0
#define ADD_VECTORS() \
  for (count = 0; count < MAX_POS; count++) *z++ = *x++ + *y++
#else
#define ADD_VECTORS() \
  for (count = 0; count < (MAX_POS+1)/2; count++,z += 2, x += 2, y += 2) \
    *(unsigned int *)z = *(unsigned int *)x + *(unsigned int *)y
#endif

  for (macroblock = 0; macroblock < mbs; macroblock++)
    {
      /* Block type 6 */
      for (block_y = 0; block_y < 2; block_y++)
	for (block_x = 0; block_x < 4; block_x++)
	  {
	    x = sads + SAD_TYPE_7_IX(mbs) +
	      macroblock * SAD_TYPE_7_CT * MAX_POS_PADDED +
	      (8 * block_y + block_x) * MAX_POS_PADDED;
	    y = x + 4 * MAX_POS_PADDED;
	    z = sads + SAD_TYPE_6_IX(mbs) +
	      macroblock * SAD_TYPE_6_CT * MAX_POS_PADDED +
	      (4 * block_y + block_x) * MAX_POS_PADDED;

	    ADD_VECTORS();
	  }

      /* Block type 5 */
      for (block_y = 0; block_y < 4; block_y++)
	for (block_x = 0; block_x < 2; block_x++)
	  {
	    x = sads + SAD_TYPE_7_IX(mbs) +
	      macroblock * SAD_TYPE_7_CT * MAX_POS_PADDED +
	      (4 * block_y + 2 * block_x) * MAX_POS_PADDED;
	    y = x + MAX_POS_PADDED;
	    z = sads + SAD_TYPE_5_IX(mbs) +
	      macroblock * SAD_TYPE_6_CT * MAX_POS_PADDED +
	      (2 * block_y + block_x) * MAX_POS_PADDED;

	    ADD_VECTORS();
	  }

      /* Block type 4 */
      for (block_y = 0; block_y < 2; block_y++)
	for (block_x = 0; block_x < 2; block_x++)
	  {
	    x = sads + SAD_TYPE_5_IX(mbs) +
	      macroblock * SAD_TYPE_5_CT * MAX_POS_PADDED +
	      (4 * block_y + block_x) * MAX_POS_PADDED;
	    y = x + 2 * MAX_POS_PADDED;
	    z = sads + SAD_TYPE_4_IX(mbs) +
	      macroblock * SAD_TYPE_4_CT * MAX_POS_PADDED +
	      (2 * block_y + block_x) * MAX_POS_PADDED;
	    
	    ADD_VECTORS();
	  }
      
      /* Block type 3 */
      x = sads + SAD_TYPE_4_IX(mbs) +
	macroblock * SAD_TYPE_4_CT * MAX_POS_PADDED;
      y = x + 2 * MAX_POS_PADDED;
      z = sads + SAD_TYPE_3_IX(mbs) +
	macroblock * SAD_TYPE_3_CT * MAX_POS_PADDED;
      
      ADD_VECTORS();

      x = sads + SAD_TYPE_4_IX(mbs) +
	macroblock * SAD_TYPE_4_CT * MAX_POS_PADDED +
	MAX_POS_PADDED;
      y = x + 2 * MAX_POS_PADDED;
      z = sads + SAD_TYPE_3_IX(mbs) +
	macroblock * SAD_TYPE_3_CT * MAX_POS_PADDED +
	MAX_POS_PADDED;

      ADD_VECTORS();

      /* Block type 2 */
      x = sads + SAD_TYPE_4_IX(mbs) +
	macroblock * SAD_TYPE_4_CT * MAX_POS_PADDED;
      y = x + MAX_POS_PADDED;
      z = sads + SAD_TYPE_2_IX(mbs) +
	macroblock * SAD_TYPE_2_CT * MAX_POS_PADDED;

      ADD_VECTORS();

      x = sads + SAD_TYPE_4_IX(mbs) +
	macroblock * SAD_TYPE_4_CT * MAX_POS_PADDED +
	2 * MAX_POS_PADDED;
      y = x + MAX_POS_PADDED;
      z = sads + SAD_TYPE_2_IX(mbs) +
	macroblock * SAD_TYPE_2_CT * MAX_POS_PADDED +
	MAX_POS_PADDED;

      ADD_VECTORS();

      /* Block type 1 */
      x = sads + SAD_TYPE_2_IX(mbs) +
	macroblock * SAD_TYPE_2_CT * MAX_POS_PADDED;
      y = x + MAX_POS_PADDED;
      z = sads + SAD_TYPE_1_IX(mbs) +
	macroblock * SAD_TYPE_1_CT * MAX_POS_PADDED;

      ADD_VECTORS();
    }
}
