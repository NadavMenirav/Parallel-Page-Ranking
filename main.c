
#include <stdio.h>
#include <stdlib.h>
#include "graph.h"
#include "pagerank.h"

static void usage(const char *prog) {
    fprintf(stderr,
            "Usage: %s <num_vertices> <num_iterations> < input_edges.txt\n",
            prog);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        usage(argv[0]);
        return 1;
    }

    char *endptr = NULL;
    long n_vertices = strtol(argv[1], &endptr, 10);
    if (*endptr != '\0' || n_vertices <= 0) {
        fprintf(stderr, "Invalid num_vertices: %s\n", argv[1]);
        return 1;
    }

    long n_iters = strtol(argv[2], &endptr, 10);
    if (*endptr != '\0' || n_iters <= 0) {
        fprintf(stderr, "Invalid num_iterations: %s\n", argv[2]);
        return 1;
    }

    Graph *g = createGraph((int)n_vertices);
    if (!g) {
        fprintf(stderr, "Failed to create graph\n");
        return 1;
    }
    
    // Read directed edges from stdin: "u v" per line.
    int u, v;
    while (scanf("%d %d", &u, &v) == 2) {
        if (u < 0 || u >= n_vertices || v < 0 || v >= n_vertices) {
            fprintf(stderr,
                    "Warning: ignoring out-of-range edge %d -> %d\n", u, v);
            continue;
        }
        addEdge(g, u, v);
    }

    // Allocate rank array and run PageRank.
    float *rank = (float *)malloc(sizeof(float) * n_vertices);
    if (!rank) {
        fprintf(stderr, "Failed to allocate rank array\n");
        return 1;
    }

    PageRank(g, (int)n_iters, rank);

    // Print final ranks, one per line: "vertex rank"
    double sum = 0.0;
    for (int i = 0; i < n_vertices; i++) {
    printf("%d %.6f\n", i, rank[i]);
    sum += rank[i];
    }
    fprintf(stderr, "Sum of ranks = %.6f\n", sum);


    free(rank);
    // We skip freeing the graph (simple test driver); OS will reclaim it.

    return 0;
}
