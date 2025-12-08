#include "graph.h"
#include "thr_pool.h"
#include <unistd.h>

#ifndef D
#define D 0.15 // The jumping factor. The probability the user will stop surfing and make a new search
#endif

typedef struct {
    float* array;
    unsigned int start;
    unsigned int end;
} ZeroArrayTask;


void PageRank(const Graph *g, int n, float* rank); // result will be returned to rank array
void zeroArray(float* array, unsigned int size, long numberOfCores); // The function zeros the array in a parallel manner
void* threadZeroArray(void* arg); // The function each thread will receive in order to zero the chunk he has


// This function will calculate the PageRank score of each node in the graph (see README)
void PageRank(const Graph *g, int n, float* rank) {

    const unsigned int N = g->numVertices; // The number of web pages
    const long numberOfCores = sysconf(_SC_NPROCESSORS_ONLN); // The amount of cores in our computer

    // We start by zeroing the rank array in parallel
    zeroArray(rank, N, numberOfCores);
}

void zeroArray(float* array, const unsigned int size, const long numberOfCores) {

    // We want to create a thread for each core we have, each thread will be given a different part of the array
    pthread_t threads[numberOfCores];
    ZeroArrayTask tasks[numberOfCores];

    // The chunk each threads will get in the array
    const int chunk = (int)(size / numberOfCores);

    for (int i = 0; i < numberOfCores; i++) {
        tasks[i].array = array;
        tasks[i].start = i * chunk;
        tasks[i].end = (i == numberOfCores - 1)? size: (i + 1) * chunk;
        pthread_create(&threads[i], NULL, &threadZeroArray, &tasks[i]);
    }

    // Barrier
    for (int i = 0; i < numberOfCores; i++) {
        pthread_join(threads[i], NULL);
    }

}

void* threadZeroArray(void* arg) {
    const ZeroArrayTask* task = (ZeroArrayTask*)arg;
    const unsigned int start = task->start;
    const unsigned int end = task->end;
    float* array = task->array;

    for (unsigned int i = start; i < end; i++) {
        array[i] = 0;
    }
    return NULL;
}


int main() {
    PageRank(NULL, 0, NULL);
    return 0;
}


