#include "graph.h"
#include "thr_pool.h"
#include <unistd.h>

#ifndef D
#define D 0.15 // The jumping factor. The probability the user will stop surfing and make a new search
#endif


void PageRank(Graph *g, int n, float* rank); // result will be returned to rank array
void zeroArray(float* array, int size);


// This function will calculate the PageRank score of each node in the graph (see README)
void PageRank(Graph *g, int n, float* rank) {
    const int N = g->numVertices; // The number of web pages
    const long numberOfCores = sysconf(_SC_NPROCESSORS_ONLN); // The amount of cores in our computer

    // We start by zeroing the rank array in parallel
}

int main() {
    PageRank(NULL, 0, NULL);
    return 0;
}


