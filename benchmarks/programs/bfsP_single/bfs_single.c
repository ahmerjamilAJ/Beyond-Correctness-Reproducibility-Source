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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>

/* --- Parboil Header Content --- */
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

/* --- Parboil Source Content --- */
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

/* --- BFS Code Content --- */

#define INF 2147483647
#define WHITE 16677217
#define GRAY 16677218
#define BLACK 16677221

typedef struct Node { int x; int y; } Node;
typedef struct Edge { int x; int y; } Edge;

int no_of_nodes;
int edge_list_size;

/* Simple Queue Implementation */
typedef struct {
    int *data;
    int head;
    int tail;
} Queue;

void queue_init(Queue *q, int size) {
    q->data = (int *)malloc(sizeof(int) * size);
    q->head = 0;
    q->tail = 0;
}
void queue_push(Queue *q, int val) { q->data[q->tail++] = val; }
int queue_pop(Queue *q) { return q->data[q->head++]; }
int queue_empty(Queue *q) { return q->head == q->tail; }
void queue_free(Queue *q) { free(q->data); }

void BFS_CPU(Node * h_graph_nodes, Edge * h_graph_edges, int * color, int * h_cost, int source) {
    Queue wavefront;
    queue_init(&wavefront, no_of_nodes);
    queue_push(&wavefront, source);
    color[source] = GRAY;
    while(!queue_empty(&wavefront)){
        int index = queue_pop(&wavefront);
        for(int i=h_graph_nodes[index].x; i<(h_graph_nodes[index].y + h_graph_nodes[index].x); i++) {
            int id = h_graph_edges[i].x;
            if(color[id] == WHITE){
                h_cost[id]=h_cost[index]+1;
                queue_push(&wavefront, id);
                color[id] = GRAY;
            }
        }
        color[index] = BLACK;
    }
    queue_free(&wavefront);
}

void runCPU( int argc, char** argv) {
    struct pb_Parameters *params;
    params = pb_ReadParameters(&argc, argv);
    if ((params->inpFiles[0] == NULL) || (params->inpFiles[1] != NULL)) {
        fprintf(stderr, "Expecting one input filename\n");
        exit(-1);
    }
    FILE *fp = fopen(params->inpFiles[0],"r");
    if(!fp) { printf("Error Reading graph file\n"); return; }
    int source;
    fscanf(fp,"%d",&no_of_nodes);
    Node* h_graph_nodes = (Node*) malloc(sizeof(Node)*no_of_nodes);
    int *color = (int*) malloc(sizeof(int)*no_of_nodes);
    int start, edgeno;   
    for( int i = 0; i < no_of_nodes; i++) {
        fscanf(fp,"%d %d",&start,&edgeno);
        h_graph_nodes[i].x = start; h_graph_nodes[i].y = edgeno;
        color[i]=WHITE;
    }
    fscanf(fp,"%d",&source);
    fscanf(fp,"%d",&edge_list_size);
    Edge* h_graph_edges = (Edge*) malloc(sizeof(Edge)*edge_list_size);
    for(int i=0; i < edge_list_size ; i++) {
        fscanf(fp,"%d %d",&h_graph_edges[i].x, &h_graph_edges[i].y);
    }
    fclose(fp);    
    int* h_cost = (int*) malloc( sizeof(int)*no_of_nodes);
    for(int i = 0; i < no_of_nodes; i++) h_cost[i] = INF;
    h_cost[source] = 0;
    BFS_CPU( h_graph_nodes, h_graph_edges, color, h_cost, source );
    /* The paper adapter exposes the complete semantic result on stdout.
       Benchmark timer text is deliberately outside the output contract. */
    printf("%d\n", no_of_nodes);
    for(int i=0;i<no_of_nodes;i++) printf("%d %d\n",i,h_cost[i]);
    free( h_graph_nodes); free( h_graph_edges); free( color); free( h_cost);
    pb_FreeParameters(params);
}

int main( int argc, char** argv) {
    runCPU(argc,argv);
    return 0;
}
