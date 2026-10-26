/* Beyond Correctness source packaging notice, 2026-10-25.
 * This is the research-adapted/amalgamated benchmark version.
 * Its upstream copyright and license notices are retained below.
 * This release adds only this notice and a line-number directive.
 * See ../../licenses/ and the benchmark catalog for notices.
 */
#line 1
/***************************************************************************
 *cr
 *cr            (C) Copyright  Illinois
 *cr
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>
#include <inttypes.h>

/* --- Parboil Header --- */
struct pb_Parameters {
  char *outFile;
  char **inpFiles;
};

struct pb_Parameters *pb_ReadParameters(int *_argc, char **argv);
void pb_FreeParameters(struct pb_Parameters *p);
int pb_Parameters_CountInputs(struct pb_Parameters *p);

typedef unsigned long long pb_Timestamp;
enum pb_TimerState { pb_Timer_STOPPED, pb_Timer_RUNNING };
struct pb_Timer {
  enum pb_TimerState state;
  pb_Timestamp elapsed;
  pb_Timestamp init;
};

void pb_ResetTimer(struct pb_Timer *timer);
void pb_StartTimer(struct pb_Timer *timer);
void pb_StopTimer(struct pb_Timer *timer);
double pb_GetElapsedTime(struct pb_Timer *timer);

enum pb_TimerID {
  pb_TimerID_NONE = 0,
  pb_TimerID_IO,
  pb_TimerID_KERNEL,
  pb_TimerID_COPY,
  pb_TimerID_DRIVER,
  pb_TimerID_COPY_ASYNC,
  pb_TimerID_COMPUTE,
  pb_TimerID_OVERLAP,
  pb_TimerID_LAST
};

struct pb_TimerSet {
  enum pb_TimerID current;
  void* async_markers;
  pb_Timestamp async_begin;
  pb_Timestamp wall_begin;
  struct pb_Timer timers[pb_TimerID_LAST];
  void *sub_timer_list[pb_TimerID_LAST];
};

void pb_InitializeTimerSet(struct pb_TimerSet *timers);
void pb_SwitchToTimer(struct pb_TimerSet *timers, enum pb_TimerID timer);
void pb_PrintTimerSet(struct pb_TimerSet *timers);

/* --- Parboil Source --- */
static void free_string_array(char **string_array) {
  char **p;
  if (!string_array) return;
  for (p = string_array; *p; p++) free(*p);
  free(string_array);
}

static char ** read_string_array(char *in) {
  char **ret;
  int i, count = 1;
  for (i = 0; in[i]; i++) if (in[i] == ',') count++;
  ret = (char **)malloc((count + 1) * sizeof(char *));
  char *substring = in;
  for (i = 0; i < count; i++) {
    char *substring_end;
    for (substring_end = substring; (*substring_end != ',') && (*substring_end != 0); substring_end++);
    int substring_length = substring_end - substring;
    ret[i] = (char *)malloc(substring_length + 1);
    memcpy(ret[i], substring, substring_length);
    ret[i][substring_length] = 0;
    substring = substring_end + 1;
  }
  ret[i] = NULL;
  return ret;
}

struct argparse { int argc; char **argv; int argn; char **argv_get; char **argv_put; };
static void initialize_argparse(struct argparse *ap, int argc, char **argv) {
  ap->argc = argc; ap->argn = 0; ap->argv_get = ap->argv_put = ap->argv = argv;
}
static void finalize_argparse(struct argparse *ap) {
  for(; ap->argn < ap->argc; ap->argn++) *ap->argv_put++ = *ap->argv_get++;
}
static void delete_argument(struct argparse *ap) { ap->argc--; ap->argv_get++; }
static void next_argument(struct argparse *ap) { *ap->argv_put++ = *ap->argv_get++; ap->argn++; }
static int is_end_of_arguments(struct argparse *ap) { return ap->argn == ap->argc; }
static char *get_argument(struct argparse *ap) { return *ap->argv_get; }
static char *consume_argument(struct argparse *ap) {
  char *ret = get_argument(ap); delete_argument(ap); return ret;
}

struct pb_Parameters * pb_ReadParameters(int *_argc, char **argv) {
  char *err_message;
  struct argparse ap;
  struct pb_Parameters *ret = (struct pb_Parameters *)malloc(sizeof(struct pb_Parameters));
  ret->outFile = NULL;
  ret->inpFiles = (char **)malloc(sizeof(char *));
  ret->inpFiles[0] = NULL;
  initialize_argparse(&ap, *_argc, argv);
  while(!is_end_of_arguments(&ap)) {
    char *arg = get_argument(&ap);
    if ((arg[0] == '-') && (arg[1] != 0) && (arg[2] == 0)) {
      delete_argument(&ap);
      switch(arg[1]) {
      case 'o':
	if (is_end_of_arguments(&ap)) { err_message = (char*)"Expecting file name after '-o'\n"; goto error; }
	free(ret->outFile); ret->outFile = strdup(consume_argument(&ap)); break;
      case 'i':
	if (is_end_of_arguments(&ap)) { err_message = (char*)"Expecting file name after '-i'\n"; goto error; }
	free_string_array(ret->inpFiles); ret->inpFiles = read_string_array(consume_argument(&ap)); break;
      case '-': goto end_of_options;
      default: err_message = (char*)"Unexpected command-line parameter\n"; goto error;
      }
    } else next_argument(&ap);
  }
 end_of_options:
  *_argc = ap.argc; finalize_argparse(&ap); return ret;
 error:
  fputs(err_message, stderr); pb_FreeParameters(ret); return NULL;
}

void pb_FreeParameters(struct pb_Parameters *p) {
  free(p->outFile); free_string_array(p->inpFiles); free(p);
}

int pb_Parameters_CountInputs(struct pb_Parameters *p) {
  int n; for (n = 0; p->inpFiles[n]; n++); return n;
}

static pb_Timestamp get_time() {
  struct timeval tv; gettimeofday(&tv, NULL);
  return (pb_Timestamp) (tv.tv_sec * 1000000LL + tv.tv_usec);
}

void pb_ResetTimer(struct pb_Timer *timer) { timer->state = pb_Timer_STOPPED; timer->elapsed = 0; }
void pb_StartTimer(struct pb_Timer *timer) {
  if (timer->state != pb_Timer_STOPPED) return;
  timer->state = pb_Timer_RUNNING;
  struct timeval tv; gettimeofday(&tv, NULL);
  timer->init = tv.tv_sec * 1000000LL + tv.tv_usec;
}
void pb_StopTimer(struct pb_Timer *timer) {
  pb_Timestamp fini;
  if (timer->state != pb_Timer_RUNNING) return;
  timer->state = pb_Timer_STOPPED;
  struct timeval tv; gettimeofday(&tv, NULL);
  fini = tv.tv_sec * 1000000LL + tv.tv_usec;
  timer->elapsed += fini - timer->init;
  timer->init = fini;
}
double pb_GetElapsedTime(struct pb_Timer *timer) { return timer->elapsed / 1e6; }

void pb_InitializeTimerSet(struct pb_TimerSet *timers) {
  int n; timers->wall_begin = get_time(); timers->current = pb_TimerID_NONE;
  for (n = 0; n < pb_TimerID_LAST; n++) { pb_ResetTimer(&timers->timers[n]); timers->sub_timer_list[n] = NULL; }
}
void pb_SwitchToTimer(struct pb_TimerSet *timers, enum pb_TimerID timer) {
  if (timers->current != pb_TimerID_NONE) pb_StopTimer(&timers->timers[timers->current]);
  timers->current = timer;
  if (timer != pb_TimerID_NONE) pb_StartTimer(&timers->timers[timer]);
}
void pb_PrintTimerSet(struct pb_TimerSet *timers) {
  pb_Timestamp wall_end = get_time();
  struct pb_Timer *t = timers->timers;
  const char *categories[] = { "IO", "Kernel", "Copy", "Driver", "Copy Async", "Compute" };
  for(int i = 1; i < pb_TimerID_LAST-1; ++i) {
    if(pb_GetElapsedTime(&t[i]) != 0) printf("%-10s: %f\n", categories[i-1], pb_GetElapsedTime(&t[i]));
  }
  printf("Timer Wall Time: %f\n", (wall_end - timers->wall_begin)/ 1e6);
}

/* --- CUTCP Headers --- */
typedef struct Atom_t { float x, y, z, q; } Atom;
typedef struct Atoms_t { Atom *atoms; int size; } Atoms;
typedef struct Vec3_t { float x, y, z; } Vec3;

typedef struct LatticeDim_t { int nx, ny, nz; Vec3 lo; float h; } LatticeDim;
typedef struct Lattice_t { LatticeDim dim; float *lattice; } Lattice;

/* --- readatom.c --- */
#define LINELEN 96
#define INITLEN 20

Atoms *read_atom_file(const char *fname) {
  FILE *file; char line[LINELEN];
  Atom *atom; int len = INITLEN; int cnt = 0;
  file = fopen(fname, "r");
  if (NULL==file) return NULL;
  atom = (Atom *) malloc(len * sizeof(Atom));
  while (fgets(line, LINELEN, file) != NULL) {
    if (strncmp(line, "ATOM  ", 6) != 0 && strncmp(line, "HETATM", 6) != 0) continue;
    if (cnt==len) {
      void *tmp = realloc(atom, 2*len*sizeof(Atom));
      atom = (Atom *) tmp; len *= 2;
    }
    if (sscanf(line, "%*s %*d %*s %*s %*d %f %f %f %f", &(atom[cnt].x), &(atom[cnt].y), &(atom[cnt].z), &(atom[cnt].q)) != 4) return NULL;
    cnt++;
  }
  fclose(file);
  Atoms *out = (Atoms *)malloc(sizeof(Atoms));
  out->size = cnt; out->atoms = atom;
  return out;
}

void free_atom(Atoms *atom) { if (atom) { free(atom->atoms); free(atom); } }

void get_atom_extent(Vec3 *out_lo, Vec3 *out_hi, Atoms *atom) {
  Atom *atoms = atom->atoms; int natoms = atom->size; Vec3 lo, hi;
  hi.x = lo.x = atoms[0].x; hi.y = lo.y = atoms[0].y; hi.z = lo.z = atoms[0].z;
  for (int n = 1; n < natoms; n++) {
    lo.x = fminf(lo.x, atoms[n].x); hi.x = fmaxf(hi.x, atoms[n].x);
    lo.y = fminf(lo.y, atoms[n].y); hi.y = fmaxf(hi.y, atoms[n].y);
    lo.z = fminf(lo.z, atoms[n].z); hi.z = fmaxf(hi.z, atoms[n].z);
  }
  *out_lo = lo; *out_hi = hi;
}

/* --- excl.c --- */
#define CELLEN      4.f
#define INV_CELLEN  (1.f/CELLEN)

int remove_exclusions(Lattice *lattice, float cutoff, Atoms *atoms) {
  int nx = lattice->dim.nx; int ny = lattice->dim.ny; int nz = lattice->dim.nz;
  float xlo = lattice->dim.lo.x; float ylo = lattice->dim.lo.y; float zlo = lattice->dim.lo.z;
  float gridspacing = lattice->dim.h; Atom *atom = atoms->atoms;
  const float a2 = cutoff * cutoff; const float inv_gridspacing = 1.f / gridspacing;
  const int radius = (int) ceilf(cutoff * inv_gridspacing) - 1;
  int n, i, j, k, ia, ib, ic, ja, jb, jc, ka, kb, kc, index, koff, jkoff;
  float x, y, z, q, dx, dy, dz, dz2, dydz2, r2, xstart, ystart, *pg;
  int gindex, ncell, nxcell, nycell, nzcell, *first, *next;
  float inv_cellen = INV_CELLEN; Vec3 minext, maxext;
  get_atom_extent(&minext, &maxext, atoms);
  nxcell = (int) floorf((maxext.x-minext.x) * inv_cellen) + 1;
  nycell = (int) floorf((maxext.y-minext.y) * inv_cellen) + 1;
  nzcell = (int) floorf((maxext.z-minext.z) * inv_cellen) + 1;
  ncell = nxcell * nycell * nzcell;
  first = (int *) malloc(ncell * sizeof(int));
  for (gindex = 0;  gindex < ncell;  gindex++) first[gindex] = -1;
  next = (int *) malloc(atoms->size * sizeof(int));
  for (n = 0;  n < atoms->size;  n++) next[n] = -1;
  for (n = 0;  n < atoms->size;  n++) {
    if (0==atom[n].q) continue;
    i = (int) floorf((atom[n].x - minext.x) * inv_cellen);
    j = (int) floorf((atom[n].y - minext.y) * inv_cellen);
    k = (int) floorf((atom[n].z - minext.z) * inv_cellen);
    gindex = (k*nycell + j)*nxcell + i;
    next[n] = first[gindex]; first[gindex] = n;
  }
  for (gindex = 0;  gindex < ncell;  gindex++) {
    for (n = first[gindex];  n != -1;  n = next[n]) {
      x = atom[n].x - xlo; y = atom[n].y - ylo; z = atom[n].z - zlo; q = atom[n].q;
      ic = (int) (x * inv_gridspacing); jc = (int) (y * inv_gridspacing); kc = (int) (z * inv_gridspacing);
      ia = ic - radius; ib = ic + radius + 1; ja = jc - radius; jb = jc + radius + 1; ka = kc - radius; kb = kc + radius + 1;
      if (ia < 0) ia = 0; if (ib >= nx) ib = nx-1; if (ja < 0) ja = 0; if (jb >= ny) jb = ny-1; if (ka < 0) ka = 0; if (kb >= nz) kb = nz-1;
      xstart = ia*gridspacing - x; ystart = ja*gridspacing - y; dz = ka*gridspacing - z;
      for (k = ka;  k <= kb;  k++, dz += gridspacing) {
        koff = k*ny; dz2 = dz*dz; dy = ystart;
        for (j = ja;  j <= jb;  j++, dy += gridspacing) {
          jkoff = (koff + j)*nx; dydz2 = dy*dy + dz2; dx = xstart; index = jkoff + ia; pg = lattice->lattice + index;
          for (i = ia;  i <= ib;  i++, pg++, dx += gridspacing) {
            r2 = dx*dx + dydz2; if (r2 < a2) *pg = 0;
          }
        }
      }
    }
  }
  free(next); free(first); return 0;
}

/* --- output.c --- */
void write_lattice_summary(const char *filename, Lattice *lattice) {
  float *lattice_data = lattice->lattice;
  int nx = lattice->dim.nx; int ny = lattice->dim.ny; int nz = lattice->dim.nz;
  FILE *outfile = fopen(filename, "wb");
  if (outfile == NULL) exit(1);
  const char magic[8] = {'C','U','T','C','P','0','0','1'};
  uint32_t dims[3] = {(uint32_t)nx, (uint32_t)ny, (uint32_t)nz};
  float geometry[4] = {lattice->dim.lo.x, lattice->dim.lo.y,
                       lattice->dim.lo.z, lattice->dim.h};
  fwrite(magic, 1, sizeof(magic), outfile);
  fwrite(dims, sizeof(uint32_t), 3, outfile);
  fwrite(geometry, sizeof(float), 4, outfile);
  fwrite(lattice_data, sizeof(float), (size_t)nx * ny * nz, outfile);
  fclose(outfile);
}

/* --- cutcpu.c --- */
#define CHECK_CYLINDER_CPU
int cpu_compute_cutoff_potential_lattice(Lattice *lattice, float cutoff, Atoms *atoms) {
  int nx = lattice->dim.nx; int ny = lattice->dim.ny; int nz = lattice->dim.nz;
  float xlo = lattice->dim.lo.x; float ylo = lattice->dim.lo.y; float zlo = lattice->dim.lo.z;
  float gridspacing = lattice->dim.h; int natoms = atoms->size; Atom *atom = atoms->atoms;
  const float a2 = cutoff * cutoff; const float inv_a2 = 1.f / a2;
  const float inv_gridspacing = 1.f / gridspacing;
  const int radius = (int) ceilf(cutoff * inv_gridspacing) - 1;
  int n, i, j, k, ia, ib, ic, ja, jb, jc, ka, kb, kc, index, koff, jkoff;
  float x, y, z, q, dx, dy, dz, dz2, dydz2, r2, s, e, xstart, ystart, *pg;
  int gindex, ncell, nxcell, nycell, nzcell, *first, *next;
  float inv_cellen = INV_CELLEN; Vec3 minext, maxext;
  get_atom_extent(&minext, &maxext, atoms);
  nxcell = (int) floorf((maxext.x-minext.x) * inv_cellen) + 1;
  nycell = (int) floorf((maxext.y-minext.y) * inv_cellen) + 1;
  nzcell = (int) floorf((maxext.z-minext.z) * inv_cellen) + 1;
  ncell = nxcell * nycell * nzcell;
  first = (int *) malloc(ncell * sizeof(int));
  for (gindex = 0;  gindex < ncell;  gindex++) first[gindex] = -1;
  next = (int *) malloc(natoms * sizeof(int));
  for (n = 0;  n < natoms;  n++) next[n] = -1;
  for (n = 0;  n < natoms;  n++) {
    if (0==atom[n].q) continue;
    i = (int) floorf((atom[n].x - minext.x) * inv_cellen);
    j = (int) floorf((atom[n].y - minext.y) * inv_cellen);
    k = (int) floorf((atom[n].z - minext.z) * inv_cellen);
    gindex = (k*nycell + j)*nxcell + i;
    next[n] = first[gindex]; first[gindex] = n;
  }
  for (gindex = 0;  gindex < ncell;  gindex++) {
    for (n = first[gindex];  n != -1;  n = next[n]) {
      x = atom[n].x - xlo; y = atom[n].y - ylo; z = atom[n].z - zlo; q = atom[n].q;
      ic = (int) (x * inv_gridspacing); jc = (int) (y * inv_gridspacing); kc = (int) (z * inv_gridspacing);
      ia = ic - radius; ib = ic + radius + 1; ja = jc - radius; jb = jc + radius + 1; ka = kc - radius; kb = kc + radius + 1;
      if (ia < 0) ia = 0; if (ib >= nx) ib = nx-1; if (ja < 0) ja = 0; if (jb >= ny) jb = ny-1; if (ka < 0) ka = 0; if (kb >= nz) kb = nz-1;
      xstart = ia*gridspacing - x; ystart = ja*gridspacing - y; dz = ka*gridspacing - z;
      for (k = ka;  k <= kb;  k++, dz += gridspacing) {
        koff = k*ny; dz2 = dz*dz; dy = ystart;
        for (j = ja;  j <= jb;  j++, dy += gridspacing) {
          jkoff = (koff + j)*nx; dydz2 = dy*dy + dz2;
#ifdef CHECK_CYLINDER_CPU
          if (dydz2 >= a2) continue;
#endif
          dx = xstart; index = jkoff + ia; pg = lattice->lattice + index;
          for (i = ia;  i <= ib;  i++, pg++, dx += gridspacing) {
            r2 = dx*dx + dydz2;
            if (r2 >= a2) continue;
            s = (1.f - r2 * inv_a2);
            e = q * (1.0f/sqrtf(r2)) * s * s;
            *pg += e;
          }
        }
      }
    }
  }
  free(next); free(first); return 0;
}

/* --- main.c --- */
LatticeDim lattice_from_bounding_box(Vec3 lo, Vec3 hi, float h) {
  LatticeDim ret;
  ret.nx = (int) floorf((hi.x-lo.x)/h) + 1;
  ret.ny = (int) floorf((hi.y-lo.y)/h) + 1;
  ret.nz = (int) floorf((hi.z-lo.z)/h) + 1;
  ret.lo = lo; ret.h = h;
  return ret;
}

Lattice * create_lattice(LatticeDim dim) {
  int size; Lattice *lat = (Lattice *)malloc(sizeof(Lattice));
  if (lat == NULL) exit(1);
  lat->dim = dim;
  size = ((dim.nx * dim.ny * dim.nz) + 7) & ~7;
  lat->lattice = (float *)calloc(size, sizeof(float));
  if (lat->lattice == NULL) exit(1);
  return lat;
}

void destroy_lattice(Lattice *lat) { if (lat) { free(lat->lattice); free(lat); } }

int main(int argc, char *argv[]) {
  Atoms *atom; LatticeDim lattice_dim; Lattice *cpu_lattice; Vec3 min_ext, max_ext; Vec3 lo, hi;
  float h = 0.5f; float cutoff = 12.f; float exclcutoff = 1.f; float padding = 0.5f;
  struct pb_Parameters *parameters; struct pb_TimerSet timers;
  parameters = pb_ReadParameters(&argc, argv);
  if (parameters == NULL) exit(1);
  if (pb_Parameters_CountInputs(parameters) != 1) { fprintf(stderr, "Expecting one input file\n"); exit(1); }
  pb_InitializeTimerSet(&timers);
  pb_SwitchToTimer(&timers, pb_TimerID_IO);
  if (!(atom = read_atom_file(parameters->inpFiles[0]))) { fprintf(stderr, "read_atom_file() failed\n"); exit(1); }
  pb_SwitchToTimer(&timers, pb_TimerID_COMPUTE);
  get_atom_extent(&min_ext, &max_ext, atom);
  lo = (Vec3) {min_ext.x - padding, min_ext.y - padding, min_ext.z - padding};
  hi = (Vec3) {max_ext.x + padding, max_ext.y + padding, max_ext.z + padding};
  lattice_dim = lattice_from_bounding_box(lo, hi, h);
  cpu_lattice = create_lattice(lattice_dim);
  if (cpu_compute_cutoff_potential_lattice(cpu_lattice, cutoff, atom)) exit(1);
  if (remove_exclusions(cpu_lattice, exclcutoff, atom)) exit(1);
  pb_SwitchToTimer(&timers, pb_TimerID_IO);
  if (parameters->outFile) write_lattice_summary(parameters->outFile, cpu_lattice);
  pb_SwitchToTimer(&timers, pb_TimerID_COMPUTE);
  destroy_lattice(cpu_lattice); free_atom(atom);
  pb_SwitchToTimer(&timers, pb_TimerID_NONE);
  pb_FreeParameters(parameters);
  return 0;
}
