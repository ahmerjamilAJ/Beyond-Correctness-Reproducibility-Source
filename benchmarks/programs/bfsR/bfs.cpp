/* Beyond Correctness source packaging notice, 2026-10-25.
 * This is the research-adapted/amalgamated benchmark version.
 * Its upstream copyright and license notices are retained below.
 * This release adds only this notice and a line-number directive.
 * See ../../licenses/ and the benchmark catalog for notices.
 */
#line 1
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

FILE *fp;

// Structure to hold a node information
struct Node {
    int starting;
    int no_of_edges;
};

void BFSGraph(int argc, char **argv);

void Usage(int argc, char **argv) {

    fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
}


////////////////////////////////////////////////////////////////////////////////
// Main Program
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv) {
    if (argc != 2) {
        Usage(argc, argv);
        return 2;
    }
    BFSGraph(argc, argv);
    return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Apply BFS on a Graph
////////////////////////////////////////////////////////////////////////////////
void BFSGraph(int argc, char **argv) {
    int no_of_nodes = 0;
    int edge_list_size = 0;
    char *input_f;

    input_f = argv[1];
    // Read in Graph from a file
    fp = fopen(input_f, "r");
    if (!fp) {
        perror("graph input");
        exit(2);
    }

    int source = 0;

    fscanf(fp, "%d", &no_of_nodes);

    // allocate host memory
    Node *h_graph_nodes = (Node *)malloc(sizeof(Node) * no_of_nodes);
    bool *h_graph_mask = (bool *)malloc(sizeof(bool) * no_of_nodes);
    bool *h_updating_graph_mask = (bool *)malloc(sizeof(bool) * no_of_nodes);
    bool *h_graph_visited = (bool *)malloc(sizeof(bool) * no_of_nodes);

    int start, edgeno;
    // initalize the memory
    for (unsigned int i = 0; i < no_of_nodes; i++) {
        fscanf(fp, "%d %d", &start, &edgeno);
        h_graph_nodes[i].starting = start;
        h_graph_nodes[i].no_of_edges = edgeno;
        h_graph_mask[i] = false;
        h_updating_graph_mask[i] = false;
        h_graph_visited[i] = false;
    }

    // read the source node from the file
    fscanf(fp, "%d", &source);
    // source=0; //tesing code line

    // set the source node as true in the mask
    h_graph_mask[source] = true;
    h_graph_visited[source] = true;

    fscanf(fp, "%d", &edge_list_size);

    int id, cost;
    int *h_graph_edges = (int *)malloc(sizeof(int) * edge_list_size);
    for (int i = 0; i < edge_list_size; i++) {
        fscanf(fp, "%d", &id);
        fscanf(fp, "%d", &cost);
        h_graph_edges[i] = id;
    }

    if (fp)
        fclose(fp);


    // allocate mem for the result on host side
    int *h_cost = (int *)malloc(sizeof(int) * no_of_nodes);
    for (int i = 0; i < no_of_nodes; i++)
        h_cost[i] = -1;
    h_cost[source] = 0;

    int k = 0;
#ifdef OPEN
#ifdef OMP_OFFLOAD
#pragma omp target data map(                                                   \
    to : no_of_nodes,                                                          \
    h_graph_mask[0 : no_of_nodes],                                             \
                 h_graph_nodes[0 : no_of_nodes], h_graph_edges                 \
                               [0 : edge_list_size], h_graph_visited           \
                                [0 : no_of_nodes],                             \
                                 h_updating_graph_mask[0 : no_of_nodes])       \
                                    map(h_cost[0 : no_of_nodes])
    {
#endif
#endif
        bool stop;
        do {
            // if no thread changes this value then the loop stops
            stop = false;

#ifdef OPEN
// omp_set_num_threads(num_omp_threads);
#ifdef OMP_OFFLOAD
#pragma omp target
#endif
#pragma omp parallel for
#endif
            for (int tid = 0; tid < no_of_nodes; tid++) {
                if (h_graph_mask[tid] == true) {
                    h_graph_mask[tid] = false;
                    for (int i = h_graph_nodes[tid].starting;
                         i < (h_graph_nodes[tid].no_of_edges +
                              h_graph_nodes[tid].starting);
                         i++) {
                        int id = h_graph_edges[i];
                        if (!h_graph_visited[id]) {
                            h_cost[id] = h_cost[tid] + 1;
                            h_updating_graph_mask[id] = true;
                        }
                    }
                }
            }

#ifdef OPEN
#ifdef OMP_OFFLOAD
#pragma omp target map(stop)
#endif
#pragma omp parallel for
#endif
            for (int tid = 0; tid < no_of_nodes; tid++) {
                if (h_updating_graph_mask[tid] == true) {
                    h_graph_mask[tid] = true;
                    h_graph_visited[tid] = true;
                    stop = true;
                    h_updating_graph_mask[tid] = false;
                }
            }
            k++;
        } while (stop);
#ifdef OPEN
#ifdef OMP_OFFLOAD
    }
#endif
#endif

    // The complete distance vector is the semantic result. The archived
    // version wrote it to output.txt; stdout lets the runner capture it
    // unambiguously for every clean and faulted trial.
    for (int i = 0; i < no_of_nodes; i++)
        printf("%d %d\n", i, h_cost[i]);

    // cleanup memory
    free(h_graph_nodes);
    free(h_graph_edges);
    free(h_graph_mask);
    free(h_updating_graph_mask);
    free(h_graph_visited);
    free(h_cost);
}
