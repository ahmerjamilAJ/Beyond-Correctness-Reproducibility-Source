/* Beyond Correctness source packaging notice, 2026-10-25.
 * This is the research-adapted/amalgamated benchmark version.
 * Its upstream copyright and license notices are retained below.
 * This release adds only this notice and a line-number directive.
 * See ../../licenses/ and the benchmark catalog for notices.
 */
#line 1
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

//#include "timer.h"

void run(int argc, char** argv);

/* define timer macros */
//#define pin_stats_reset()   startCycle()
//#define pin_stats_pause(cycles)   stopCycle(cycles)
//#define pin_stats_dump(cycles)    printf("timer: %Lu\n", cycles)

int rows, cols;
int* data;
int** wall;
int* result;

void
init(int argc, char** argv)
{
	if (argc != 2) {
        fprintf(stderr, "usage: pathfinder INPUT\n");
        exit(2);
    }
    FILE *input = fopen(argv[1], "r");
    if (!input) {
        perror("grid input");
        exit(2);
    }
    if (fscanf(input, "%d %d", &rows, &cols) != 2 ||
        rows < 1 || cols < 1 || rows > 128 || cols > 128) {
        fprintf(stderr, "invalid grid dimensions\n");
        exit(2);
    }
	data = new int[rows*cols];
	wall = new int*[rows];
	for(int n=0; n<rows; n++)
		wall[n]=data+cols*n;
	result = new int[cols];
	
	for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            if (fscanf(input, "%d", &wall[i][j]) != 1 ||
                wall[i][j] < 0 || wall[i][j] > 99) {
                fprintf(stderr, "invalid grid cell\n");
                exit(2);
            }
        }
    }
    int extra;
    if (fscanf(input, "%d", &extra) == 1) {
        fprintf(stderr, "trailing grid cells\n");
        exit(2);
    }
    fclose(input);
    for (int j = 0; j < cols; j++)
        result[j] = wall[0][j];
}

void 
fatal(char *s)
{
	fprintf(stderr, "error: %s\n", s);

}

#define IN_RANGE(x, min, max)   ((x)>=(min) && (x)<=(max))
#define CLAMP_RANGE(x, min, max) x = (x<(min)) ? min : ((x>(max)) ? max : x )
#define MIN(a, b) ((a)<=(b) ? (a) : (b))

int main(int argc, char** argv)
{
    run(argc,argv);

    return EXIT_SUCCESS;
}

void run(int argc, char** argv)
{
    init(argc, argv);

    int *src, *dst, *temp;
    int min;

    dst = result;
    src = new int[cols];

    //pin_stats_reset();
    for (int t = 0; t < rows-1; t++) {
        temp = src;
        src = dst;
        dst = temp;
        #pragma omp parallel for private(min)
        for(int n = 0; n < cols; n++){
          min = src[n];
          if (n > 0)
            min = MIN(min, src[n-1]);
          if (n < cols-1)
            min = MIN(min, src[n+1]);
          dst[n] = wall[t+1][n]+min;
        }
    }

    //pin_stats_pause(cycles);
    //pin_stats_dump(cycles);

    for (int i = 0; i < cols; i++)
            printf("%d%c", dst[i], i + 1 == cols ? '\n' : ' ');

    delete [] data;
    delete [] wall;
    delete [] dst;
    delete [] src;
}
