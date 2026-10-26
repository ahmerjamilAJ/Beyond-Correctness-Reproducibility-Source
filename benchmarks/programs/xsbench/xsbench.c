/* Beyond Correctness source packaging notice, 2026-10-25.
 * This is the research-adapted/amalgamated benchmark version.
 * Its upstream copyright and license notices are retained below.
 * This release adds only this notice and a line-number directive.
 * See ../../licenses/ and the benchmark catalog for notices.
 */
#line 1
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<string.h>
#include<strings.h>
#include<math.h>
#include<stdint.h>
#ifdef OPENMP
#include<omp.h>
#endif
#include<unistd.h>
#include<sys/time.h>

// Papi Header
#ifdef PAPI
#include "papi.h"
#endif

#ifdef MPI
#include<mpi.h>
#endif

// I/O Specifiers
#define INFO 0
#define DEBUG 0
#define SAVE 0
#define printf(...) ((int)0)
#define fputs(...) ((int)0)

// HASAN start
#define VERIFICATION
// HASAN end

// Structures
typedef struct{
	double energy;
	double total_xs;
	double elastic_xs;
	double absorbtion_xs;
	double fission_xs;
	double nu_fission_xs;
} NuclideGridPoint;

typedef struct{
	double energy;
	int * xs_ptrs;
} GridPoint;

typedef struct{
	int nthreads;
	long n_isotopes;
	long n_gridpoints;
	int lookups;
	char * HM;
} Inputs;

// Function Prototypes
void logo(int version);
void center_print(const char *s, int width);
void border_print(void);
void fancy_int(long a);

NuclideGridPoint ** gpmatrix(size_t m, size_t n);

void gpmatrix_free( NuclideGridPoint ** M );

int NGP_compare( const void * a, const void * b );

void generate_grids( NuclideGridPoint ** nuclide_grids,
                     long n_isotopes, long n_gridpoints );
void generate_grids_v( NuclideGridPoint ** nuclide_grids,
                     long n_isotopes, long n_gridpoints );

void sort_nuclide_grids( NuclideGridPoint ** nuclide_grids, long n_isotopes,
                         long n_gridpoints );

GridPoint * generate_energy_grid( long n_isotopes, long n_gridpoints,
                                  NuclideGridPoint ** nuclide_grids);

void set_grid_ptrs( GridPoint * energy_grid, NuclideGridPoint ** nuclide_grids,
                    long n_isotopes, long n_gridpoints );

int binary_search( NuclideGridPoint * A, double quarry, int n );

void calculate_macro_xs(   double p_energy, int mat, long n_isotopes,
                           long n_gridpoints, int * restrict num_nucs,
                           double ** restrict concs,
						   GridPoint * restrict energy_grid,
                           NuclideGridPoint ** restrict nuclide_grids,
						   int ** restrict mats,
                           double * restrict macro_xs_vector );

void calculate_micro_xs(   double p_energy, int nuc, long n_isotopes,
                           long n_gridpoints,
                           GridPoint * restrict energy_grid,
                           NuclideGridPoint ** restrict nuclide_grids, int idx,
                           double * restrict xs_vector );

long grid_search( long n, double quarry, GridPoint * A);

int * load_num_nucs(long n_isotopes);
int ** load_mats( int * num_nucs, long n_isotopes );
double ** load_concs( int * num_nucs );
double ** load_concs_v( int * num_nucs );
int pick_mat(unsigned long * seed);
double rn(unsigned long * seed);
int rn_int(unsigned long * seed);
void counter_stop( int * eventset, int num_papi_events );
void counter_init( int * eventset, int * num_papi_events );
void do_flops(void);
void do_loads( int nuc,
               NuclideGridPoint ** restrict nuclide_grids,
		       long n_gridpoints );
Inputs read_CLI( int argc, char * argv[] );
void print_CLI_error(void);
double rn_v(void);
double round_double( double input );
unsigned int hash(unsigned char *str, int nbins);
size_t estimate_mem_usage( Inputs in );
void print_inputs(Inputs in, int nprocs, int version);
void print_results( Inputs in, int mype, double runtime, int nprocs, unsigned long long vhash );
void binary_dump(long n_isotopes, long n_gridpoints, NuclideGridPoint ** nuclide_grids, GridPoint * energy_grid);
void binary_read(long n_isotopes, long n_gridpoints, NuclideGridPoint ** nuclide_grids, GridPoint * energy_grid);

// =================================================================================
// Utility Functions (formerly XSutils.c)
// =================================================================================

// Allocates nuclide matrix
NuclideGridPoint ** gpmatrix(size_t m, size_t n)
{
	int i,j;
	NuclideGridPoint * full = (NuclideGridPoint *) malloc( m * n *
	                          sizeof( NuclideGridPoint ) );
	NuclideGridPoint ** M = (NuclideGridPoint **) malloc( m *
	                          sizeof(NuclideGridPoint *) );

	for( i = 0, j=0; i < m*n; i++ )
		if( i % n == 0 )
			M[j++] = &full[i];

	return M;
}

// Frees nuclide matrix
void gpmatrix_free( NuclideGridPoint ** M )
{
	free( *M );
	free( M );
}

// Compare function for two grid points. Used for sorting during init
int NGP_compare( const void * a, const void * b )
{
	NuclideGridPoint *i, *j;

	i = (NuclideGridPoint *) a;
	j = (NuclideGridPoint *) b;

	if( i->energy > j->energy )
		return 1;
	else if ( i->energy < j->energy)
		return -1;
	else
		return 0;
}

// Binary Search function for nuclide grid
// Returns ptr to energy less than the quarry that is closest to the quarry
int binary_search( NuclideGridPoint * A, double quarry, int n )
{
	int min = 0;
	int max = n-1;
	int mid;
	
	// checks to ensure we're not reading off the end of the grid
	if( A[0].energy > quarry )
		return 0;
	else if( A[n-1].energy < quarry )
		return n-2;
	
	// Begins binary search	
	while( max >= min )
	{
		mid = min + floor( (max-min) / 2.0);
		if( A[mid].energy < quarry )
			min = mid+1;
		else if( A[mid].energy > quarry )
			max = mid-1;
		else
			return mid;
	}
	return max;
}

// Park & Miller Multiplicative Conguential Algorithm
// From "Numerical Recipes" Second Edition
double rn(unsigned long * seed)
{
	double ret;
	unsigned long n1;
	unsigned long a = 16807;
	unsigned long m = 2147483647;
	n1 = ( a * (*seed) ) % m;
	*seed = n1;
	ret = (double) n1 / m;
	return ret;
}

// RNG Used for Verification Option.
// This one has a static seed (must be set manually in source).
// Park & Miller Multiplicative Conguential Algorithm
// From "Numerical Recipes" Second Edition
double rn_v(void)
{
	static unsigned long seed = 1337;
	double ret;
	unsigned long n1;
	unsigned long a = 16807;
	unsigned long m = 2147483647;
	n1 = ( a * (seed) ) % m;
	seed = n1;
	ret = (double) n1 / m;
	return ret;
}

unsigned int hash(unsigned char *str, int nbins)
{
	unsigned int hash = 5381;
	int c;

	while (c = *str++)
		hash = ((hash << 5) + hash) + c;

	return hash % nbins;
}

size_t estimate_mem_usage( Inputs in )
{
	size_t single_nuclide_grid = in.n_gridpoints * sizeof( NuclideGridPoint );
	size_t all_nuclide_grids   = in.n_isotopes * single_nuclide_grid;
	size_t size_GridPoint      = sizeof(GridPoint) + in.n_isotopes*sizeof(int);
	size_t size_UEG            = in.n_isotopes*in.n_gridpoints * size_GridPoint;
	size_t memtotal;

	memtotal          = all_nuclide_grids + size_UEG;
	all_nuclide_grids = all_nuclide_grids / 1048576;
	size_UEG          = size_UEG / 1048576;
	memtotal          = memtotal / 1048576;
	return memtotal;
}

void binary_dump(long n_isotopes, long n_gridpoints, NuclideGridPoint ** nuclide_grids, GridPoint * energy_grid)
{
	FILE * fp = fopen("XS_data.dat", "wb");
	// Dump Nuclide Grid Data
	for( long i = 0; i < n_isotopes; i++ )
		fwrite(nuclide_grids[i], sizeof(NuclideGridPoint), n_gridpoints, fp);
	// Dump UEG Data
	for( long i = 0; i < n_isotopes * n_gridpoints; i++ )
	{
		// Write energy level
		fwrite(&energy_grid[i].energy, sizeof(double), 1, fp);

		// Write index data array (xs_ptrs array)
		fwrite(energy_grid[i].xs_ptrs, sizeof(int), n_isotopes, fp);
	}

	fclose(fp);
}

void binary_read(long n_isotopes, long n_gridpoints, NuclideGridPoint ** nuclide_grids, GridPoint * energy_grid)
{
	int stat;
	FILE * fp = fopen("XS_data.dat", "rb");
	// Read Nuclide Grid Data
	for( long i = 0; i < n_isotopes; i++ )
		stat = fread(nuclide_grids[i], sizeof(NuclideGridPoint), n_gridpoints, fp);
	// Dump UEG Data
	for( long i = 0; i < n_isotopes * n_gridpoints; i++ )
	{
		// Write energy level
		stat = fread(&energy_grid[i].energy, sizeof(double), 1, fp);

		// Write index data array (xs_ptrs array)
		stat = fread(energy_grid[i].xs_ptrs, sizeof(int), n_isotopes, fp);
	}

	fclose(fp);
}

// =================================================================================
// Material Data Functions (formerly Materials.c)
// =================================================================================

// Material data is hard coded into the functions in this file.
// Note that there are 12 materials present in H-M (large or small)

// num_nucs represents the number of nuclides that each material contains
int * load_num_nucs(long n_isotopes)
{
	int * num_nucs = (int*)malloc(12*sizeof(int));
	
	// Material 0 is a special case (fuel). The H-M small reactor uses
	// 34 nuclides, while H-M larges uses 300.
	if( n_isotopes == 68 )
		num_nucs[0]  = 34; // HM Small is 34, H-M Large is 321
	else
		num_nucs[0]  = 321; // HM Small is 34, H-M Large is 321

	num_nucs[1]  = 5;
	num_nucs[2]  = 4;
	num_nucs[3]  = 4;
	num_nucs[4]  = 27;
	num_nucs[5]  = 21;
	num_nucs[6]  = 21;
	num_nucs[7]  = 21;
	num_nucs[8]  = 21;
	num_nucs[9]  = 21;
	num_nucs[10] = 9;
	num_nucs[11] = 9;

	return num_nucs;
}

// Assigns an array of nuclide ID's to each material
int ** load_mats( int * num_nucs, long n_isotopes )
{
	int ** mats = (int **) malloc( 12 * sizeof(int *) );
	for( int i = 0; i < 12; i++ )
		mats[i] = (int *) malloc(num_nucs[i] * sizeof(int) );

	// Small H-M has 34 fuel nuclides
	int mats0_Sml[] =  { 58, 59, 60, 61, 40, 42, 43, 44, 45, 46, 1, 2, 3, 7,
	                 8, 9, 10, 29, 57, 47, 48, 0, 62, 15, 33, 34, 52, 53, 
	                 54, 55, 56, 18, 23, 41 }; //fuel
	// Large H-M has 300 fuel nuclides
	int mats0_Lrg[321] =  { 58, 59, 60, 61, 40, 42, 43, 44, 45, 46, 1, 2, 3, 7,
	                 8, 9, 10, 29, 57, 47, 48, 0, 62, 15, 33, 34, 52, 53,
	                 54, 55, 56, 18, 23, 41 }; //fuel
	for( int i = 0; i < 321-34; i++ )
		mats0_Lrg[34+i] = 68 + i; // H-M large adds nuclides to fuel only
	
	// These are the non-fuel materials	
	int mats1[] =  { 63, 64, 65, 66, 67 }; // cladding
	int mats2[] =  { 24, 41, 4, 5 }; // cold borated water
	int mats3[] =  { 24, 41, 4, 5 }; // hot borated water
	int mats4[] =  { 19, 20, 21, 22, 35, 36, 37, 38, 39, 25, 27, 28, 29,
	                 30, 31, 32, 26, 49, 50, 51, 11, 12, 13, 14, 6, 16,
	                 17 }; // RPV
	int mats5[] =  { 24, 41, 4, 5, 19, 20, 21, 22, 35, 36, 37, 38, 39, 25,
	                 49, 50, 51, 11, 12, 13, 14 }; // lower radial reflector
	int mats6[] =  { 24, 41, 4, 5, 19, 20, 21, 22, 35, 36, 37, 38, 39, 25,
	                 49, 50, 51, 11, 12, 13, 14 }; // top reflector / plate
	int mats7[] =  { 24, 41, 4, 5, 19, 20, 21, 22, 35, 36, 37, 38, 39, 25,
	                 49, 50, 51, 11, 12, 13, 14 }; // bottom plate
	int mats8[] =  { 24, 41, 4, 5, 19, 20, 21, 22, 35, 36, 37, 38, 39, 25,
	                 49, 50, 51, 11, 12, 13, 14 }; // bottom nozzle
	int mats9[] =  { 24, 41, 4, 5, 19, 20, 21, 22, 35, 36, 37, 38, 39, 25,
	                 49, 50, 51, 11, 12, 13, 14 }; // top nozzle
	int mats10[] = { 24, 41, 4, 5, 63, 64, 65, 66, 67 }; // top of FA's
	int mats11[] = { 24, 41, 4, 5, 63, 64, 65, 66, 67 }; // bottom FA's
	
	// H-M large v small dependency
	if( n_isotopes == 68 )
		memcpy( mats[0],  mats0_Sml,  num_nucs[0]  * sizeof(int) );	
	else
		memcpy( mats[0],  mats0_Lrg,  num_nucs[0]  * sizeof(int) );
	
	// Copy other materials
	memcpy( mats[1],  mats1,  num_nucs[1]  * sizeof(int) );	
	memcpy( mats[2],  mats2,  num_nucs[2]  * sizeof(int) );	
	memcpy( mats[3],  mats3,  num_nucs[3]  * sizeof(int) );	
	memcpy( mats[4],  mats4,  num_nucs[4]  * sizeof(int) );	
	memcpy( mats[5],  mats5,  num_nucs[5]  * sizeof(int) );	
	memcpy( mats[6],  mats6,  num_nucs[6]  * sizeof(int) );	
	memcpy( mats[7],  mats7,  num_nucs[7]  * sizeof(int) );	
	memcpy( mats[8],  mats8,  num_nucs[8]  * sizeof(int) );	
	memcpy( mats[9],  mats9,  num_nucs[9]  * sizeof(int) );	
	memcpy( mats[10], mats10, num_nucs[10] * sizeof(int) );	
	memcpy( mats[11], mats11, num_nucs[11] * sizeof(int) );	
	
	return mats;
}

// Creates a randomized array of 'concentrations' of nuclides in each mat
double ** load_concs( int * num_nucs )
{
	double ** concs = (double **)malloc( 12 * sizeof( double *) );
	
	for( int i = 0; i < 12; i++ )
		concs[i] = (double *)malloc( num_nucs[i] * sizeof(double) );
	
	// HASAN start
	for( int i = 0; i < 12; i++ )
		for( int j = 0; j < num_nucs[i]; j++ )
			concs[i][j] = 114.00; //(double) rand() / (double) RAND_MAX;

	// HASAN end

	return concs;
}

// Verification version of this function (tighter control over RNG)
double ** load_concs_v( int * num_nucs )
{
	double ** concs = (double **)malloc( 12 * sizeof( double *) );
	
	for( int i = 0; i < 12; i++ )
		concs[i] = (double *)malloc( num_nucs[i] * sizeof(double) );
	
	for( int i = 0; i < 12; i++ )
		for( int j = 0; j < num_nucs[i]; j++ )
			concs[i][j] = rn_v();

	return concs;
}

// picks a material based on a probabilistic distribution
int pick_mat( unsigned long * seed )
{
	// I have a nice spreadsheet supporting these numbers. They are
	// the fractions (by volume) of material in the core. Not a 
	// *perfect* approximation of where XS lookups are going to occur,
	// but this will do a good job of biasing the system nonetheless.

	// Also could be argued that doing fractions by weight would be 
	// a better approximation, but volume does a good enough job for now.

	double dist[12];
	dist[0]  = 0.140;	// fuel
	dist[1]  = 0.052;	// cladding
	dist[2]  = 0.275;	// cold, borated water
	dist[3]  = 0.134;	// hot, borated water
	dist[4]  = 0.154;	// RPV
	dist[5]  = 0.064;	// Lower, radial reflector
	dist[6]  = 0.066;	// Upper reflector / top plate
	dist[7]  = 0.055;	// bottom plate
	dist[8]  = 0.008;	// bottom nozzle
	dist[9]  = 0.015;	// top nozzle
	dist[10] = 0.025;	// top of fuel assemblies
	dist[11] = 0.013;	// bottom of fuel assemblies
	
	//double roll = (double) rand() / (double) RAND_MAX;
	#ifdef VERIFICATION
	double roll = rn_v();
	#else
	double roll = rn(seed);
	#endif

	// makes a pick based on the distro
	for( int i = 0; i < 12; i++ )
	{
		double running = 0;
		for( int j = i; j > 0; j-- )
			running += dist[j];
		if( roll < running )
			return i;
	}

	return 0;
}

// =================================================================================
// Grid Initialization Functions (formerly GridInit.c)
// =================================================================================

// Generates randomized energy grid for each nuclide
// Note that this is done as part of initialization (serial), so
// rand() is used.
void generate_grids( NuclideGridPoint ** nuclide_grids,
                     long n_isotopes, long n_gridpoints ) {
	for( long i = 0; i < n_isotopes; i++ )
		for( long j = 0; j < n_gridpoints; j++ )
		{
			// HASAN start
			nuclide_grids[i][j].energy       = 324.00; //((double)rand()/(double)RAND_MAX);
			nuclide_grids[i][j].total_xs     = 520.00; //((double)rand()/(double)RAND_MAX);
			nuclide_grids[i][j].elastic_xs   = 345.00; // ((double)rand()/(double)RAND_MAX);
			nuclide_grids[i][j].absorbtion_xs= 326.00; // ((double)rand()/(double)RAND_MAX);
			nuclide_grids[i][j].fission_xs   = 456.00; //((double)rand()/(double)RAND_MAX);
			nuclide_grids[i][j].nu_fission_xs= 512.00; // ((double)rand()/(double)RAND_MAX);
			// HASAN end
		}
}

// Verification version of this function (tighter control over RNG)
void generate_grids_v( NuclideGridPoint ** nuclide_grids,
                     long n_isotopes, long n_gridpoints ) {
	for( long i = 0; i < n_isotopes; i++ )
		for( long j = 0; j < n_gridpoints; j++ )
		{
			nuclide_grids[i][j].energy       = rn_v();
			nuclide_grids[i][j].total_xs     = rn_v();
			nuclide_grids[i][j].elastic_xs   = rn_v();
			nuclide_grids[i][j].absorbtion_xs= rn_v();
			nuclide_grids[i][j].fission_xs   = rn_v();
			nuclide_grids[i][j].nu_fission_xs= rn_v();
		}
}

// Sorts the nuclide grids by energy (lowest -> highest)
void sort_nuclide_grids( NuclideGridPoint ** nuclide_grids, long n_isotopes,
                         long n_gridpoints )
{
	int (*cmp) (const void *, const void *);
	cmp = NGP_compare;

	for( long i = 0; i < n_isotopes; i++ )
		qsort( nuclide_grids[i], n_gridpoints, sizeof(NuclideGridPoint),
		       cmp );
}

// Allocates unionized energy grid, and assigns union of energy levels
// from nuclide grids to it.
GridPoint * generate_energy_grid( long n_isotopes, long n_gridpoints,
                                  NuclideGridPoint ** nuclide_grids) {
	int mype = 0;

	#ifdef MPI
	MPI_Comm_rank(MPI_COMM_WORLD, &mype);
	#endif

	if( mype == 0 ) printf("Generating Unionized Energy Grid...\n");

	long n_unionized_grid_points = n_isotopes*n_gridpoints;
	int (*cmp) (const void *, const void *);
	cmp = NGP_compare;

	GridPoint * energy_grid = (GridPoint *)malloc( n_unionized_grid_points
	                                               * sizeof( GridPoint ) );
	if( mype == 0 ) printf("Copying and Sorting all nuclide grids...\n");

	NuclideGridPoint ** n_grid_sorted = gpmatrix( n_isotopes, n_gridpoints );


	memcpy( n_grid_sorted[0], nuclide_grids[0], n_isotopes*n_gridpoints*
	                                      sizeof( NuclideGridPoint ) );

	qsort( &n_grid_sorted[0][0], n_unionized_grid_points,
	       sizeof(NuclideGridPoint), cmp);

	if( mype == 0 ) printf("Assigning energies to unionized grid...\n");

	for( long i = 0; i < n_unionized_grid_points; i++ )
		energy_grid[i].energy = n_grid_sorted[0][i].energy;


	gpmatrix_free(n_grid_sorted);

	int * full = (int *) malloc( n_isotopes * n_unionized_grid_points
	                             * sizeof(int) );
	if( full == NULL )
	{
		fprintf(stderr,"ERROR - Out Of Memory!\n");
		exit(1);
	}

	for( long i = 0; i < n_unionized_grid_points; i++ )
		energy_grid[i].xs_ptrs = &full[n_isotopes * i];

	return energy_grid;
}

// Searches each nuclide grid for the closest energy level and assigns
// pointer from unionized grid to the correct spot in the nuclide grid.
// This process is time consuming, as the number of binary searches
// required is:  binary searches = n_gridpoints * n_isotopes^2
void set_grid_ptrs( GridPoint * energy_grid, NuclideGridPoint ** nuclide_grids,
                    long n_isotopes, long n_gridpoints )
{
	int mype = 0;

	#ifdef MPI
	MPI_Comm_rank(MPI_COMM_WORLD, &mype);
	#endif

	if( mype == 0 ) printf("Assigning pointers to Unionized Energy Grid...\n");
	#pragma omp parallel for default(none) \
	shared( energy_grid, nuclide_grids, n_isotopes, n_gridpoints, mype )
	for( long i = 0; i < n_isotopes * n_gridpoints ; i++ )
	{
		double quarry = energy_grid[i].energy;
#ifdef OPENMP
		if( INFO && mype == 0 && omp_get_thread_num() == 0 && i % 200 == 0 )
#else
		if( INFO && mype == 0 && i % 200 == 0 )
#endif
			printf("\rAligning Unionized Grid...(%.0lf%% complete)",
			       100.0 * (double) i / (n_isotopes*n_gridpoints /
#ifdef OPENMP
				                         omp_get_num_threads()
#else
							 1
#endif
							 )
			       );
		for( long j = 0; j < n_isotopes; j++ )
		{
			// j is the nuclide i.d.
			// log n binary search
			energy_grid[i].xs_ptrs[j] =
				binary_search( nuclide_grids[j], quarry, n_gridpoints);
		}
	}
	if( mype == 0 ) printf("\n");
}

// =================================================================================
// Cross Section Calculation Functions (formerly CalculateXS.c)
// =================================================================================

// Calculates the microscopic cross section for a given nuclide & energy
void calculate_micro_xs(   double p_energy, int nuc, long n_isotopes,
                           long n_gridpoints,
                           GridPoint * restrict energy_grid,
                           NuclideGridPoint ** restrict nuclide_grids,
                           int idx, double * restrict xs_vector ){
	
	// Variables
	double f;
	NuclideGridPoint * low, * high;

	// pull ptr from energy grid and check to ensure that
	// we're not reading off the end of the nuclide's grid
	if( energy_grid[idx].xs_ptrs[nuc] == n_gridpoints - 1 )
		low = &nuclide_grids[nuc][energy_grid[idx].xs_ptrs[nuc] - 1];
	else
		low = &nuclide_grids[nuc][energy_grid[idx].xs_ptrs[nuc]];
	
	high = low + 1;
	
	// calculate the re-useable interpolation factor
	f = (high->energy - p_energy) / (high->energy - low->energy);

	// Total XS
	xs_vector[0] = high->total_xs - f * (high->total_xs - low->total_xs);
	
	// Elastic XS
	xs_vector[1] = high->elastic_xs - f * (high->elastic_xs - low->elastic_xs);
	
	// Absorbtion XS
	xs_vector[2] = high->absorbtion_xs - f * (high->absorbtion_xs - low->absorbtion_xs);
	
	// Fission XS
	xs_vector[3] = high->fission_xs - f * (high->fission_xs - low->fission_xs);
	
	// Nu Fission XS
	xs_vector[4] = high->nu_fission_xs - f * (high->nu_fission_xs - low->nu_fission_xs);
}

// Calculates macroscopic cross section based on a given material & energy 
void calculate_macro_xs( double p_energy, int mat, long n_isotopes,
                         long n_gridpoints, int * restrict num_nucs,
                         double ** restrict concs,
                         GridPoint * restrict energy_grid,
                         NuclideGridPoint ** restrict nuclide_grids,
                         int ** restrict mats,
                         double * restrict macro_xs_vector ){
	double xs_vector[5];
	int p_nuc; // the nuclide we are looking up
	long idx = 0;	
	double conc; // the concentration of the nuclide in the material

	// cleans out macro_xs_vector
	for( int k = 0; k < 5; k++ )
		macro_xs_vector[k] = 0;

	// binary search for energy on unionized energy grid (UEG)
	idx = grid_search( n_isotopes * n_gridpoints, p_energy,
	                   energy_grid);	
	
	// Once we find the pointer array on the UEG, we can pull the data
	// from the respective nuclide grids, as well as the nuclide
	// concentration data for the material
	// Each nuclide from the material needs to have its micro-XS array
	// looked up & interpolatied (via calculate_micro_xs). Then, the
	// micro XS is multiplied by the concentration of that nuclide
	// in the material, and added to the total macro XS array.
	for( int j = 0; j < num_nucs[mat]; j++ )
	{
		p_nuc = mats[mat][j];
		conc = concs[mat][j];
		calculate_micro_xs( p_energy, p_nuc, n_isotopes,
		                    n_gridpoints, energy_grid,
		                    nuclide_grids, idx, xs_vector );
		for( int k = 0; k < 5; k++ ){
			macro_xs_vector[k] += xs_vector[k] * conc;
		}
	}
}

// (fixed) binary search for energy on unionized energy grid
// returns lower index
long grid_search( long n, double quarry, GridPoint * A)
{
	long lowerLimit = 0;
	long upperLimit = n-1;
	long examinationPoint;
	long length = upperLimit - lowerLimit;

	while( length > 1 )
	{
		examinationPoint = lowerLimit + ( length / 2 );
		
		if( A[examinationPoint].energy > quarry )
			upperLimit = examinationPoint;
		else
			lowerLimit = examinationPoint;
		
		length = upperLimit - lowerLimit;
	}
	
	return lowerLimit;
}

// =================================================================================
// I/O and CLI Parsing Functions (formerly io.c)
// =================================================================================

// Prints program logo
void logo(int version)
{
	border_print();
	printf(
	"                   __   __ ___________                 _                        \n"
	"                   \\ \\ / //  ___| ___ \\               | |                       \n"
	"                    \\ V / \\ `--.| |_/ / ___ _ __   ___| |__                     \n"
	"                    /   \\  `--. \\ ___ \\/ _ \\ '_ \\ / __| '_ \\                    \n"
	"                   / /^\\ \\/\\__/ / |_/ /  __/ | | | (__| | | |                   \n"
	"                   \\/   \\/\\____/\\____/ \\___|_| |_|\\___|_| |_|                   \n\n"
	);
	border_print();
	center_print("Developed at Argonne National Laboratory", 79);
	char v[100];
	sprintf(v, "Version: %d", version);
	center_print(v, 79);
	border_print();
}

// Prints Section titles in center of 80 char terminal
void center_print(const char *s, int width)
{
	int length = strlen(s);
	int i;
	for (i=0; i<=(width-length)/2; i++) {
		fputs(" ", stdout);
	}
	fputs(s, stdout);
	fputs("\n", stdout);
}

void print_results( Inputs in, int mype, double runtime, int nprocs,
	unsigned long long vhash )
{
	// Calculate Lookups per sec
	int lookups_per_sec = (int) ((double) in.lookups / runtime);

	// If running in MPI, reduce timing statistics and calculate average
	#ifdef MPI
	int total_lookups = 0;
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Reduce(&lookups_per_sec, &total_lookups, 1, MPI_INT,
	           MPI_SUM, 0, MPI_COMM_WORLD);
	#endif

	// Print output
	if( mype == 0 )
	{
		border_print();
		center_print("RESULTS", 79);
		border_print();

		// Print the results
		printf("Threads:     %d\n", in.nthreads);
		#ifdef MPI
		printf("MPI ranks:   %d\n", nprocs);
		#endif
		#ifdef MPI
		printf("Total Lookups/s:            ");
		fancy_int(total_lookups);
		printf("Avg Lookups/s per MPI rank: ");
		fancy_int(total_lookups / nprocs);
		#else
		// printf("Runtime:     %.3lf seconds\n", runtime);
		printf("Lookups:     "); fancy_int(in.lookups);
		// printf("Lookups/s:   ");
		// fancy_int(lookups_per_sec);
		#endif
		#ifdef VERIFICATION
		printf("Verification checksum: %llu\n", vhash);
		#endif
		border_print();

		// For bechmarking, output lookup/s data to file
		if( SAVE )
		{
			FILE * out = fopen( "results.txt", "a" );
			fprintf(out, "%d\t%d\n", in.nthreads, lookups_per_sec);
			fclose(out);
		}
	}
}

void print_inputs(Inputs in, int nprocs, int version )
{
	// Calculate Estimate of Memory Usage
	int mem_tot = estimate_mem_usage( in );
	logo(version);
	center_print("INPUT SUMMARY", 79);
	border_print();
	#ifdef VERIFICATION
	printf("Verification Mode:            on\n");
	#endif
	printf("Materials:                    %d\n", 12);
	printf("H-M Benchmark Size:           %s\n", in.HM);
	printf("Total Nuclides:               %ld\n", in.n_isotopes);
	printf("Gridpoints (per Nuclide):     ");
	fancy_int(in.n_gridpoints);
	printf("Unionized Energy Gridpoints:  ");
	fancy_int(in.n_isotopes*in.n_gridpoints);
	printf("XS Lookups:                   "); fancy_int(in.lookups);
	#ifdef MPI
	printf("MPI Ranks:                    %d\n", nprocs);
	printf("OMP Threads per MPI Rank:     %d\n", in.nthreads);
	printf("Mem Usage per MPI Rank (MB):  "); fancy_int(mem_tot);
	#else
	printf("Threads:                      %d\n", in.nthreads);
	printf("Est. Memory Usage (MB):       "); fancy_int(mem_tot);
	#endif
	border_print();
	center_print("INITIALIZATION", 79);
	border_print();
}

void border_print(void)
{
	printf(
	"==================================================================="
	"=============\n");
}

// Prints comma separated integers - for ease of reading
void fancy_int( long a )
{
    if( a < 1000 )
        printf("%ld\n",a);

    else if( a >= 1000 && a < 1000000 )
        printf("%ld,%03ld\n", a / 1000, a % 1000);

    else if( a >= 1000000 && a < 1000000000 )
        printf("%ld,%03ld,%03ld\n",a / 1000000,(a % 1000000) / 1000,a % 1000 );

    else if( a >= 1000000000 )
        printf("%ld,%03ld,%03ld,%03ld\n",
               a / 1000000000,
               (a % 1000000000) / 1000000,
               (a % 1000000) / 1000,
               a % 1000 );
    else
        printf("%ld\n",a);
}

void print_CLI_error(void)
{
	printf("Usage: ./XSBench <options>\n");
	printf("Options include:\n");
	printf("  -t <threads>     Number of OpenMP threads to run\n");
	printf("  -s <size>        Size of H-M Benchmark to run (small, large, XL, XXL)\n");
	printf("  -g <gridpoints>  Number of gridpoints per nuclide (overrides -s defaults)\n");
	printf("  -l <lookups>     Number of Cross-section (XS) lookups\n");
	printf("Default is equivalent to: -s large -l 15000000\n");
	printf("See readme for full description of default run values\n");
	exit(4);
}

Inputs read_CLI( int argc, char * argv[] )
{
	Inputs input;

#ifdef OPENMP
	// defaults to max threads on the system
	input.nthreads = omp_get_num_procs();
#else
	input.nthreads = 1;
#endif

	// defaults to 355 (corresponding to H-M Large benchmark)
	input.n_isotopes = 355;

	// defaults to 11303 (corresponding to H-M Large benchmark)
	input.n_gridpoints = 11303;

	// defaults to 15,000,000
	input.lookups = 15000000;

	// defaults to H-M Large benchmark
	input.HM = (char *) malloc( 6 * sizeof(char) );
	input.HM[0] = 'l' ;
	input.HM[1] = 'a' ;
	input.HM[2] = 'r' ;
	input.HM[3] = 'g' ;
	input.HM[4] = 'e' ;
	input.HM[5] = '\0';

	// Check if user sets these
	int user_g = 0;

	// Collect Raw Input
	for( int i = 1; i < argc; i++ )
	{
		char * arg = argv[i];

		// nthreads (-t)
		if( strcmp(arg, "-t") == 0 )
		{
			if( ++i < argc )
				input.nthreads = atoi(argv[i]);
			else
				print_CLI_error();
		}
		// n_gridpoints (-g)
		else if( strcmp(arg, "-g") == 0 )
		{
			if( ++i < argc )
			{
				user_g = 1;
				input.n_gridpoints = atol(argv[i]);
			}
			else
				print_CLI_error();
		}
		// lookups (-l)
		else if( strcmp(arg, "-l") == 0 )
		{
			if( ++i < argc )
				input.lookups = atoi(argv[i]);
			else
				print_CLI_error();
		}
		// HM (-s)
		else if( strcmp(arg, "-s") == 0 )
		{
			if( ++i < argc )
				input.HM = argv[i];
			else
				print_CLI_error();
		}
		else
			print_CLI_error();
	}

	// Validate Input

	// Validate nthreads
	if( input.nthreads < 1 )
		print_CLI_error();

	// Validate n_isotopes
	if( input.n_isotopes < 1 )
		print_CLI_error();

	// Validate n_gridpoints
	if( input.n_gridpoints < 1 )
		print_CLI_error();

	// Validate lookups
	if( input.lookups < 1 )
		print_CLI_error();

	// Validate HM size
	if( strcasecmp(input.HM, "small") != 0 &&
		strcasecmp(input.HM, "large") != 0 &&
		strcasecmp(input.HM, "XL") != 0 &&
		strcasecmp(input.HM, "XXL") != 0 )
		print_CLI_error();

	// Set HM size specific parameters
	// (defaults to large)
	if( strcasecmp(input.HM, "small") == 0 )
		input.n_isotopes = 68;
	else if( strcasecmp(input.HM, "XL") == 0 && user_g == 0 )
		input.n_gridpoints = 238847; // sized to make 120 GB XS data
	else if( strcasecmp(input.HM, "XXL") == 0 && user_g == 0 )
		input.n_gridpoints = 238847 * 2.1; // 252 GB XS data

	// Return input struct
	return input;
}

// =================================================================================
// Main and Time Functions (formerly Main.c)
// =================================================================================

double get_wtime()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);

	double ret = tv.tv_sec + tv.tv_usec/1e6;

	return  ret;
}

int main( int argc, char* argv[] )
{
	// =====================================================================
	// Initialization & Command Line Read-In
	// =====================================================================
	int version = 13;
	int mype = 0;
#ifdef OPENMP
	int max_procs = omp_get_num_procs();
#endif
	int i, thread, mat;
	unsigned long seed;
	double omp_start, omp_end, p_energy;
	unsigned long long vhash = 0;
	int nprocs = 1;

	#ifdef MPI
	MPI_Status stat;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &mype);
	#endif

	// rand() is only used in the serial initialization stages.
	// A custom RNG is used in parallel portions.
	// HASAN start
	#ifdef VERIFICATION
	//srand(26);
	#else
	//srand(time(NULL));
	#endif
	// HASAN end 

	// Process CLI Fields -- store in "Inputs" structure
	Inputs in = read_CLI( argc, argv );

#ifdef OPENMP
	// Set number of OpenMP Threads
	omp_set_num_threads(in.nthreads);
#endif

	// Print-out of Input Summary
	if( mype == 0 )
		print_inputs( in, nprocs, version );

	// =====================================================================
	// Prepare Nuclide Energy Grids, Unionized Energy Grid, & Material Data
	// =====================================================================

	// Allocate & fill energy grids
	#ifndef BINARY_READ
	if( mype == 0) printf("Generating Nuclide Energy Grids...\n");
	#endif

	NuclideGridPoint ** nuclide_grids = gpmatrix(in.n_isotopes,in.n_gridpoints);

	#ifdef VERIFICATION
	generate_grids_v( nuclide_grids, in.n_isotopes, in.n_gridpoints );
	#else
	generate_grids( nuclide_grids, in.n_isotopes, in.n_gridpoints );
	#endif

	// Sort grids by energy
	#ifndef BINARY_READ
	if( mype == 0) printf("Sorting Nuclide Energy Grids...\n");
	sort_nuclide_grids( nuclide_grids, in.n_isotopes, in.n_gridpoints );
	#endif

	// Prepare Unionized Energy Grid Framework
	#ifndef BINARY_READ
	GridPoint * energy_grid = generate_energy_grid( in.n_isotopes,
	                          in.n_gridpoints, nuclide_grids );
	#else
	GridPoint * energy_grid = (GridPoint *)malloc( in.n_isotopes *
	                           in.n_gridpoints * sizeof( GridPoint ) );
	int * index_data = (int *) malloc( in.n_isotopes * in.n_gridpoints
	                   * in.n_isotopes * sizeof(int));
	for( i = 0; i < in.n_isotopes*in.n_gridpoints; i++ )
		energy_grid[i].xs_ptrs = &index_data[i*in.n_isotopes];
	#endif

	// Double Indexing. Filling in energy_grid with pointers to the
	// nuclide_energy_grids.
	#ifndef BINARY_READ
	set_grid_ptrs( energy_grid, nuclide_grids, in.n_isotopes, in.n_gridpoints );
	#endif

	#ifdef BINARY_READ
	if( mype == 0 ) printf("Reading data from \"XS_data.dat\" file...\n");
	binary_read(in.n_isotopes, in.n_gridpoints, nuclide_grids, energy_grid);
	#endif

	// Get material data
	if( mype == 0 )
		printf("Loading Mats...\n");
	int *num_nucs  = load_num_nucs(in.n_isotopes);
	int **mats     = load_mats(num_nucs, in.n_isotopes);

	#ifdef VERIFICATION
	double **concs = load_concs_v(num_nucs);
	#else
	double **concs = load_concs(num_nucs);
	#endif

	#ifdef BINARY_DUMP
	if( mype == 0 ) printf("Dumping data to binary file...\n");
	binary_dump(in.n_isotopes, in.n_gridpoints, nuclide_grids, energy_grid);
	if( mype == 0 ) printf("Binary file \"XS_data.dat\" written! Exiting...\n");
	return 0;
	#endif

	// =====================================================================
	// Cross Section (XS) Parallel Lookup Simulation Begins
	// =====================================================================

#ifdef OPENMP
	// Outer benchmark loop can loop through all possible # of threads
	#ifdef BENCHMARK
	for( int bench_n = 1; bench_n <=omp_get_num_procs(); bench_n++ )
	{
		in.nthreads = bench_n;
		omp_set_num_threads(in.nthreads);
  #endif
#endif

	if( mype == 0 )
	{
		printf("\n");
		border_print();
		center_print("SIMULATION", 79);
		border_print();
	}

#ifdef OPENMP
	omp_start = omp_get_wtime();
#else
	omp_start = get_wtime();
#endif

	//initialize papi with one thread (master) here
	#ifdef PAPI
	if ( PAPI_library_init(PAPI_VER_CURRENT) != PAPI_VER_CURRENT){
		fprintf(stderr, "PAPI library init error!\n");
		exit(1);
	}
	#endif

	double *trace_energy = (double *)calloc((size_t)in.lookups, sizeof(double));
	int32_t *trace_material = (int32_t *)calloc((size_t)in.lookups, sizeof(int32_t));
	double *trace_xs = (double *)calloc((size_t)in.lookups * 5, sizeof(double));
	if (!trace_energy || !trace_material || !trace_xs) exit(1);

	// OpenMP compiler directives - declaring variables as shared or private
	#pragma omp parallel default(none) \
	private(i, thread, p_energy, mat, seed) \
	shared( in, energy_grid, nuclide_grids, \
	        mats, concs, num_nucs, mype, vhash)
	{
		// Initialize parallel PAPI counters
		#ifdef PAPI
		int eventset = PAPI_NULL;
		int num_papi_events;
		#pragma omp critical
		{
			counter_init(&eventset, &num_papi_events);
		}
		#endif

		double macro_xs_vector[5];
		double * xs = (double *) calloc(5, sizeof(double));

		// Initialize RNG seeds for threads
#ifdef OPENMP
		thread = omp_get_thread_num();
#else
		thread = 0;
#endif
		// HASAN start
		//seed   = (thread+1)*19+17;
		
		seed = 36;
		// HASAN end

		// XS Lookup Loop
		#pragma omp for schedule(dynamic)
		for( i = 0; i < in.lookups; i++ )
		{
			// Status text
			if( INFO && mype == 0 && thread == 0 && i % 1000 == 0 )
				printf("\rCalculating XS's... (%.0lf%% completed)",
						(i / ( (double)in.lookups / (double) in.nthreads ))
						/ (double) in.nthreads * 100.0);

			// Randomly pick an energy and material for the particle
			#ifdef VERIFICATION
			#pragma omp critical
			{
				p_energy = rn_v();
				mat      = pick_mat(&seed);
			}
			#else
			p_energy = rn(&seed);
			mat      = pick_mat(&seed);
			#endif

			// This returns the macro_xs_vector, but we're not going
			// to do anything with it in this program, so return value
			// is written over.
			calculate_macro_xs( p_energy, mat, in.n_isotopes,
			                    in.n_gridpoints, num_nucs, concs,
			                    energy_grid, nuclide_grids, mats,
                                macro_xs_vector );

			// Copy results from above function call onto heap
			// so that compiler cannot optimize function out
			// (only occurs if -flto flag is used)
			memcpy(xs, macro_xs_vector, 5*sizeof(double));
			trace_energy[i] = p_energy;
			trace_material[i] = (int32_t)mat;
			memcpy(&trace_xs[(size_t)i * 5], macro_xs_vector, 5*sizeof(double));

			// Verification hash calculation
			// This method provides a consistent hash accross
			// architectures and compilers.
			#ifdef VERIFICATION
			char line[256];
			sprintf(line, "%.5lf %d %.5lf %.5lf %.5lf %.5lf %.5lf",
			       p_energy, mat,
				   macro_xs_vector[0],
				   macro_xs_vector[1],
				   macro_xs_vector[2],
				   macro_xs_vector[3],
				   macro_xs_vector[4]);
			
			unsigned long long vhash_local = hash(line, 10000);
			#pragma omp atomic
			vhash += vhash_local;
			#endif

		}

		// Prints out thread local PAPI counters
		#ifdef PAPI
		if( mype == 0 && thread == 0 )
		{
			printf("\n");
			border_print();
			center_print("PAPI COUNTER RESULTS", 79);
			border_print();
			printf("Count          \tSmybol      \tDescription\n");
		}
		{
		#pragma omp barrier
		}
		counter_stop(&eventset, num_papi_events);
		#endif

		free(xs);
	}

	#ifndef PAPI
	if( mype == 0)
	{
		printf("\n" );
		printf("Simulation complete.\n" );
	}
	#endif

#ifdef OPENMP
	omp_end = omp_get_wtime();
#else
	omp_end = get_wtime();
#endif

	// Print / Save Results and Exit
	print_results( in, mype, omp_end-omp_start, nprocs, vhash );
	FILE *trace_file = fopen("result.bin", "wb");
	if (!trace_file) exit(1);
	const char trace_magic[8] = {'X','S','B','E','N','C','H','1'};
	uint64_t trace_shape[3] = {(uint64_t)in.n_isotopes,
	                           (uint64_t)in.n_gridpoints,
	                           (uint64_t)in.lookups};
	fwrite(trace_magic, 1, sizeof(trace_magic), trace_file);
	fwrite(trace_shape, sizeof(uint64_t), 3, trace_file);
	for (i = 0; i < in.lookups; ++i) {
		fwrite(&trace_energy[i], sizeof(double), 1, trace_file);
		fwrite(&trace_material[i], sizeof(int32_t), 1, trace_file);
		fwrite(&trace_xs[(size_t)i * 5], sizeof(double), 5, trace_file);
	}
	fclose(trace_file);

	#ifdef BENCHMARK
	}
	#endif

	#ifdef MPI
	MPI_Finalize();
	#endif

	free(trace_xs);
	free(trace_material);
	free(trace_energy);
	free(energy_grid[0].xs_ptrs);
	free(energy_grid);
	gpmatrix_free(nuclide_grids);
	for (i = 0; i < 12; ++i) { free(mats[i]); free(concs[i]); }
	free(mats);
	free(concs);
	free(num_nucs);
	free(in.HM);

	return 0;
}
