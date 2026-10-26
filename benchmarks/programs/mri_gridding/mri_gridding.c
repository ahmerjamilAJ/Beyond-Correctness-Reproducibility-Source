/* Beyond Correctness source packaging notice, 2026-10-25.
 * This is the research-adapted/amalgamated benchmark version.
 * Its upstream copyright and license notices are retained below.
 * This release adds only this notice and a line-number directive.
 * See ../../licenses/ and the benchmark catalog for notices.
 */
#line 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdint.h>

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
double pb_GetElapsedTime(struct pb_Timer *timer) { return (double)timer->elapsed / 1e6; }

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
  printf("Timer Wall Time: %f\n", (double)(wall_end - timers->wall_begin)/ 1e6);
}

/* --- MRI Types --- */
typedef struct{ int numSamples; int aquisitionMatrixSize[3]; int reconstructionMatrixSize[3]; float kMax[3]; int gridSize[3]; float oversample; float kernelWidth; int binsize; int useLUT; } parameters;
typedef struct{ float real; float imag; float kX; float kY; float kZ; float sdc; } ReconstructionSample;
typedef struct{ float real; float imag; } cmplx;

/* --- CPU Kernels --- */
float kernel_value_CPU(float v){
  float rVal = 0;
  float v2 = v*v;
  if (v > 0){
    rVal = sinh(sqrt(v2))/sqrt(v2);
  } else {
    rVal = 1;
  }
  return rVal;
}

float kernel_value_LUT(float v, float* LUT, int sizeLUT, float _1overCutoff2){
  float rVal = 0;
  int idx = (int)(v*_1overCutoff2*sizeLUT);
  if (idx >= 0 && idx < sizeLUT){
    rVal = LUT[idx];
  }
  return rVal;
}

void calculateLUT(float beta, float width, float** LUT, unsigned int* sizeLUT){
  float v;
  float cutoff2 = (width/2.0)*(width/2.0);
  float _1overCutoff2 = 1.0/cutoff2;
  *sizeLUT = 10000;
  *LUT = (float*) malloc ((*sizeLUT)*sizeof(float));
  for(int i=0; i<(*sizeLUT); i++){
    v = (float)i/((float)(*sizeLUT)*_1overCutoff2);
    (*LUT)[i] = kernel_value_CPU(beta*sqrt(1.0-(v*_1overCutoff2)));
  }
}

void gridding_Gold(unsigned int n, parameters params, ReconstructionSample* samples, float* LUT, unsigned int sizeLUT, cmplx* gridData, float* sampleDensity){
  float cutoff = params.kernelWidth/2.0;
  float cutoff2 = cutoff*cutoff;
  float _1overCutoff2 = 1.0/cutoff2;
  float beta = 3.14159265 * sqrt(4*params.kernelWidth*params.kernelWidth/(params.oversample*params.oversample) * (params.oversample-.5)*(params.oversample-.5)-.8);
  int size_x = params.gridSize[0]; int size_y = params.gridSize[1]; int size_z = params.gridSize[2];
  for(unsigned int i=0; i<n; i++){
    ReconstructionSample pt = samples[i];
    int NxL = (int)ceil(pt.kX - cutoff); int NxH = (int)floor(pt.kX + cutoff);
    int NyL = (int)ceil(pt.kY - cutoff); int NyH = (int)floor(pt.kY + cutoff);
    int NzL = (int)ceil(pt.kZ - cutoff); int NzH = (int)floor(pt.kZ + cutoff);
    if (NxL < 0) NxL = 0; if (NxH >= size_x) NxH = size_x-1;
    if (NyL < 0) NyL = 0; if (NyH >= size_y) NyH = size_y-1;
    if (NzL < 0) NzL = 0; if (NzH >= size_z) NzH = size_z-1;
    for(int nz=NzL; nz<=NzH; nz++){
      float dz = (float)nz-pt.kZ; float dz2 = dz*dz;
      for(int ny=NyL; ny<=NyH; ny++){
        float dy = (float)ny-pt.kY; float dydz2 = dy*dy + dz2;
        if(dydz2 < cutoff2){
          for(int nx=NxL; nx<=NxH; nx++){
            float dx = (float)nx-pt.kX; float v = dx*dx + dydz2;
            if(v < cutoff2){
              int idx = (nz*size_y + ny)*size_x + nx;
              float w = (params.useLUT ? kernel_value_LUT(v, LUT, sizeLUT, _1overCutoff2) : kernel_value_CPU(beta*sqrt(1.0-(v*_1overCutoff2)))) * pt.sdc;
              gridData[idx].real += (w*pt.real); gridData[idx].imag += (w*pt.imag);
              sampleDensity[idx] += 1.0;
            }
          }
        }
      }
    }
  }
}

/* --- Main --- */
int main (int argc, char* argv[]){
  struct pb_Parameters* prms; struct pb_TimerSet timers;
  prms = pb_ReadParameters(&argc,argv);
  pb_InitializeTimerSet(&timers);
  char uksdata[250]; parameters params;
  strcpy(uksdata,prms->inpFiles[0]); strcat(uksdata,".data");
  FILE* uksfile_f = fopen(prms->inpFiles[0],"r");
  if (!uksfile_f) exit(1);
  params.binsize = (argc >= 2) ? atoi(argv[1]) : 128;
  fscanf(uksfile_f,"aquisition.numsamples=%d\n",&(params.numSamples));
  fscanf(uksfile_f,"aquisition.kmax=%f %f %f\n",&(params.kMax[0]), &(params.kMax[1]), &(params.kMax[2]));
  fscanf(uksfile_f,"aquisition.matrixSize=%d %d %d\n", &(params.aquisitionMatrixSize[0]), &(params.aquisitionMatrixSize[1]), &(params.aquisitionMatrixSize[2]));
  fscanf(uksfile_f,"reconstruction.matrixSize=%d %d %d\n", &(params.reconstructionMatrixSize[0]), &(params.reconstructionMatrixSize[1]), &(params.reconstructionMatrixSize[2]));
  fscanf(uksfile_f,"gridding.matrixSize=%d %d %d\n", &(params.gridSize[0]), &(params.gridSize[1]), &(params.gridSize[2]));
  fscanf(uksfile_f,"gridding.oversampling=%f\n", &(params.oversample));
  fscanf(uksfile_f,"kernel.width=%f\n", &(params.kernelWidth));
  fscanf(uksfile_f,"kernel.useLUT=%d\n", &(params.useLUT));
  fclose(uksfile_f);
  pb_SwitchToTimer(&timers, pb_TimerID_IO);
  ReconstructionSample* samples = (ReconstructionSample*) malloc (params.numSamples*sizeof(ReconstructionSample));
  float* LUT; unsigned int sizeLUT;
  int gridNumElems = params.gridSize[0]*params.gridSize[1]*params.gridSize[2];
  cmplx* gridData = (cmplx*) calloc (gridNumElems, sizeof(cmplx));
  float* sampleDensity = (float*) calloc (gridNumElems, sizeof(float));
  FILE* uksdata_f = fopen(uksdata,"rb");
  if(!uksdata_f) exit(1);
  unsigned int i;
  for(i=0; i<params.numSamples; i++){
    if (feof(uksdata_f)) break;
    fread((void*) &(samples[i]), sizeof(ReconstructionSample), 1, uksdata_f);
  }
  fclose(uksdata_f);
  float kScale[3];
  kScale[0] = (float)(params.aquisitionMatrixSize[0])/((float)(params.reconstructionMatrixSize[0])*(float)(params.kMax[0]));
  kScale[1] = (float)(params.aquisitionMatrixSize[1])/((float)(params.reconstructionMatrixSize[1])*(float)(params.kMax[1]));
  kScale[2] = (float)(params.aquisitionMatrixSize[2])/((float)(params.reconstructionMatrixSize[2])*(float)(params.kMax[2]));
  float ax = (kScale[0]*(params.gridSize[0]-1))/2.0; float bx = (float)(params.gridSize[0]-1)/2.0;
  float ay = (kScale[1]*(params.gridSize[1]-1))/2.0; float by = (float)(params.gridSize[1]-1)/2.0;
  float az = (kScale[2]*(params.gridSize[2]-1))/2.0; float bz = (float)(params.gridSize[2]-1)/2.0;
  for(int n=0; n<i; n++){
    samples[n].kX = floor((samples[n].kX*ax)+bx); samples[n].kY = floor((samples[n].kY*ay)+by); samples[n].kZ = floor((samples[n].kZ*az)+bz);
  }
  if (params.useLUT){
    float beta = 3.14159265 * sqrt(4*params.kernelWidth*params.kernelWidth/(params.oversample*params.oversample) * (params.oversample-.5)*(params.oversample-.5)-.8);
    calculateLUT(beta, params.kernelWidth, &LUT, &sizeLUT);
  }
  pb_SwitchToTimer(&timers, pb_TimerID_COMPUTE);
  gridding_Gold(i, params, samples, LUT, sizeLUT, gridData, sampleDensity);
  pb_SwitchToTimer(&timers, pb_TimerID_IO);
  if(prms->outFile){
    FILE* outfile = fopen(prms->outFile,"wb");
    if(!outfile) exit(1);
    const char magic[8] = {'M','R','I','G','R','I','D','1'};
    uint32_t dims[3] = {(uint32_t)params.gridSize[0], (uint32_t)params.gridSize[1],
                        (uint32_t)params.gridSize[2]};
    fwrite(magic,1,sizeof(magic),outfile);
    fwrite(dims,sizeof(uint32_t),3,outfile);
    fwrite(gridData,sizeof(cmplx),gridNumElems,outfile);
    fwrite(sampleDensity,sizeof(float),gridNumElems,outfile);
    fclose(outfile);
  }
  pb_SwitchToTimer(&timers, pb_TimerID_NONE);
  if (params.useLUT) free(LUT);
  free(samples); free(gridData); free(sampleDensity);
  pb_FreeParameters(prms);
  return 0;
}
