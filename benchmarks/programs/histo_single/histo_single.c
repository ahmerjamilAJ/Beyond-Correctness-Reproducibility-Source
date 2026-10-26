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
void pb_AddSubTimer(struct pb_TimerSet *timers, char *label, enum pb_TimerID pb_Category);
void pb_SwitchToSubTimer(struct pb_TimerSet *timers, char *label, enum pb_TimerID category);

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
	free_string_array(ret->inpFiles);
	ret->inpFiles = read_string_array(consume_argument(&ap)); break;
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

typedef struct pb_SubTimer {
  char *label;
  struct pb_Timer timer;
  struct pb_SubTimer *next;
} pb_SubTimer;

typedef struct pb_SubTimerList {
  pb_SubTimer *current;
  pb_SubTimer *subtimer_list;
} pb_SubTimerList;

void pb_AddSubTimer(struct pb_TimerSet *timers, char *label, enum pb_TimerID pb_Category) {
  pb_SubTimer *subtimer = (pb_SubTimer *) malloc(sizeof(pb_SubTimer));
  subtimer->label = strdup(label);
  pb_ResetTimer(&subtimer->timer);
  subtimer->next = NULL;
  pb_SubTimerList *subtimerlist = (pb_SubTimerList *)timers->sub_timer_list[pb_Category];
  if (subtimerlist == NULL) {
    subtimerlist = (pb_SubTimerList *) malloc(sizeof(pb_SubTimerList));
    subtimerlist->subtimer_list = subtimer;
    subtimerlist->current = NULL;
    timers->sub_timer_list[pb_Category] = subtimerlist;
  } else {
    pb_SubTimer *element = subtimerlist->subtimer_list;
    while (element->next != NULL) element = element->next;
    element->next = subtimer;
  }
}

void pb_SwitchToTimer(struct pb_TimerSet *timers, enum pb_TimerID timer) {
  if (timers->current != pb_TimerID_NONE) {
    pb_StopTimer(&timers->timers[timers->current]);
    pb_SubTimerList *list = (pb_SubTimerList *)timers->sub_timer_list[timers->current];
    if (list && list->current) pb_StopTimer(&list->current->timer);
  }
  timers->current = timer;
  if (timer != pb_TimerID_NONE) pb_StartTimer(&timers->timers[timer]);
}

void pb_SwitchToSubTimer(struct pb_TimerSet *timers, char *label, enum pb_TimerID category) {
  if (timers->current != pb_TimerID_NONE) {
    pb_StopTimer(&timers->timers[timers->current]);
    pb_SubTimerList *list = (pb_SubTimerList *)timers->sub_timer_list[timers->current];
    if (list && list->current) pb_StopTimer(&list->current->timer);
  }
  timers->current = category;
  if (category != pb_TimerID_NONE) {
    pb_StartTimer(&timers->timers[category]);
    pb_SubTimerList *list = (pb_SubTimerList *)timers->sub_timer_list[category];
    if (list) {
      pb_SubTimer *sub = list->subtimer_list;
      while (sub) {
        if (strcmp(sub->label, label) == 0) { list->current = sub; break; }
        sub = sub->next;
      }
      if (list->current) pb_StartTimer(&list->current->timer);
    }
  }
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

/* --- BMP Content --- */
typedef struct { unsigned char B; unsigned char G; unsigned char R; } RGB;
typedef struct { unsigned int filesz; unsigned short creator1; unsigned short creator2; unsigned int bmp_offset; } bmpfile_header_t;
typedef struct { unsigned int header_sz; unsigned int width; unsigned int height; unsigned short nplanes; unsigned short bitspp; unsigned int compress_type; unsigned int bmp_bytesz; unsigned int hres; unsigned int vres; unsigned int ncolors; unsigned int nimpcolors; } bmp_dib_header_t;
typedef struct { unsigned char magic[2]; bmpfile_header_t file_header; bmp_dib_header_t dib_header; unsigned int* palette; void* pixel_map; } bmp_image;

void create_bmp(RGB* bitmap, int height, int width, const char* filename){
    bmp_image image;
    int padded_width = 4*(((width*24)+31)/32);
    padded_width -= width*sizeof(RGB);
    char* pad = (char*) calloc (padded_width, sizeof(char));
    image.magic[0]='B'; image.magic[1]='M';
    image.file_header.filesz = 2*sizeof(char) + sizeof(bmpfile_header_t) + sizeof(bmp_dib_header_t) + height*width*sizeof(RGB);
    image.file_header.creator1 = image.file_header.creator2 = 0;
    image.file_header.bmp_offset = 2*sizeof(char) + sizeof(bmpfile_header_t) + sizeof(bmp_dib_header_t);
    image.dib_header.header_sz = 40; image.dib_header.width = width; image.dib_header.height = height;
    image.dib_header.nplanes = 1; image.dib_header.bitspp = 24; image.dib_header.compress_type = 0;
    image.dib_header.bmp_bytesz = width*height*sizeof(RGB);
    image.dib_header.hres = 0; image.dib_header.vres = 0; image.dib_header.ncolors = 0; image.dib_header.nimpcolors = 0;
    FILE* out_file = fopen(filename,"wb");
    fwrite(image.magic,sizeof(char),2,out_file);
    fwrite(&(image.file_header),sizeof(char),sizeof(bmpfile_header_t),out_file);
    fwrite(&(image.dib_header),sizeof(char),sizeof(bmp_dib_header_t),out_file);
    for (int h = height-1; h >= 0; h--){
      fwrite(&bitmap[h*width],sizeof(RGB),width,out_file);
      fwrite(pad,sizeof(char),padded_width,out_file);
    }
    fclose(out_file); free(pad);
}

/* --- Util Content --- */
RGB HSVtoRGB( float h, float s, float v ) {
    int i; float f, p, q, t, r, g, b; RGB value={0,0,0};
    if( s == 0 ) { r = g = b = v; return value; }
    h /= 60; i = floor( h ); f = h - i; p = v * ( 1 - s ); q = v * ( 1 - s * f ); t = v * ( 1 - s * ( 1 - f ) );
    switch( i ) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        default: r = v; g = p; b = q; break;
    }
    value.R = r*255; value.G = g*255; value.B = b*255;
    return value;
}

void dump_histo_img(unsigned char* histo, unsigned int height, unsigned int width, const char *filename) {
    RGB* pixel_map = (RGB*) malloc (height*width*sizeof(RGB));
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            unsigned char value = histo[y * width + x];
            if (value == 0){ pixel_map[y*width+x].R = pixel_map[y*width+x].G = pixel_map[y*width+x].B = 0; }
            else { pixel_map[y*width+x] = HSVtoRGB(0.0,1.0,cbrt(1+ 63.0*((float)value)/255.0)/4); }
        }
    }
    create_bmp(pixel_map, height, width, filename);
    free(pixel_map);
}

/* --- Main Content --- */
int main(int argc, char* argv[]) {
  struct pb_Parameters *parameters;
  parameters = pb_ReadParameters(&argc, argv);
  if (!parameters || !parameters->inpFiles[0]) return -1;
  int numIterations = (argc >= 2) ? atoi(argv[1]) : 1;
  unsigned int img_width, img_height, histo_width, histo_height;
  FILE* f = fopen(parameters->inpFiles[0],"rb");
  if (!f) return -1;
  fread(&img_width, sizeof(unsigned int), 1, f);
  fread(&img_height, sizeof(unsigned int), 1, f);
  fread(&histo_width, sizeof(unsigned int), 1, f);
  fread(&histo_height, sizeof(unsigned int), 1, f);
  unsigned int* img = (unsigned int*) malloc (img_width*img_height*sizeof(unsigned int));
  unsigned char* histo = (unsigned char*) calloc (histo_width*histo_height, sizeof(unsigned char));
  fread(img, sizeof(unsigned int), img_width*img_height, f);
  fclose(f);
  for (int iter = 0; iter < numIterations; iter++){
    memset(histo, 0, histo_height*histo_width*sizeof(unsigned char));
    for (unsigned int i = 0; i < img_width*img_height; ++i) {
      const unsigned int value = img[i];
      if (value < histo_width*histo_height && histo[value] < 255) { ++histo[value]; }
    }
  }
  printf("%u %u\n", histo_width, histo_height);
  for (unsigned int i = 0; i < histo_width * histo_height; ++i)
    printf("%u\n", (unsigned int)histo[i]);
  free(img); free(histo);
  pb_FreeParameters(parameters);
  return 0;
}
