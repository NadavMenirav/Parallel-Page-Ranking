#include "graph.h"
#include "thr_pool.h"
#include <unistd.h>

#ifndef D
#define D 0.85f // The damping factor. The probability the user will continue to surf and click links
#endif

typedef struct {
    float* array;
    size_t start;
    size_t end;
    float value;
} InitArrayTask;

typedef struct {
    size_t* array;
    size_t index; // Maybe change to range of indices
    node* outlinks;
} CountOutlinks;

typedef struct {
    float* temp; // The array the thread will write the new result to
    float* array; // The array of the previous PageRanks
    size_t* outlinksCount; // The number of outlinks for every node in the graph
    size_t index; // Which node it calculates
    node* outlinks; // The adjacency list
    const Graph* graph; // The graph
} CalculatePageRank;

// result will be returned to rank array
void PageRank(const Graph *g, int n, float* rank);

// The function initializes the array in a parallel manner
void initArray(float* array, size_t size, long numberOfCores, float value);

// The function each thread will receive in order to initialize the chunk he has
void* threadInitArray(void* arg);

// This function will be used to improve our current estimation of the rank of all the vertices
void improve(const Graph* g, thr_pool_t* pool, float* array, size_t* outlinks, size_t size, long numberOfCores);

// This function will fill an array with the outlinks for each vertex
void getOutlinks(const Graph* g, size_t* result, size_t size, long numberOfCores);

// The function each thread will receive in order to count the number of outlinks each vertex has
void* threadGetOutlinks(void* arg);

// The function each thread will receive in order to calculate the page rank
void* threadCalculatePageRank(void* arg);


// This function will calculate the PageRank score of each node in the graph (see README)
void PageRank(const Graph *g, const int n, float* rank) {
    if (!g) // CLion cried for not doing it
        return;

    const size_t N = g->numVertices; // The number of web pages
    const long numberOfCores = sysconf(_SC_NPROCESSORS_ONLN); // The amount of cores in our computer

    /*
     * We start by assigning 1/N as the rank for each vertex
     * (We will use n iterations that will improve this first assignment)
     */
    initArray(rank, N, numberOfCores, 1.f / (float)N);

    // We want to create an array that for each vertex store the number of OutLinks he has
    size_t* outlinks = malloc(N * sizeof(size_t));
    if (!outlinks) exit(-1);

    // Now we call getOutlinks to fill the outlinks array
    getOutlinks(g, outlinks, N, numberOfCores);

    // We create the thread pool that the "improve" function will queue tasks for calculating the PR to
    thr_pool_t* pool = thr_pool_create(numberOfCores, numberOfCores, 0, NULL);

    for (int i = 0; i < n; i++) {

        // Enqueue the tasks
        improve(g, pool, rank, outlinks, N, numberOfCores);
    }

    thr_pool_destroy(pool);


    free(outlinks);
}

void initArray(float* array, const size_t size, const long numberOfCores, const float value) {

    // We want to create a thread for each core we have, each thread will be given a different part of the array
    pthread_t* threads = malloc(sizeof(pthread_t) * numberOfCores);
    if (!threads) exit(-1);
    InitArrayTask* tasks = malloc(sizeof(InitArrayTask) * numberOfCores);
    if (!tasks) exit(-1);

    // The chunk each threads will get in the array
    const size_t chunk = size / numberOfCores;

    for (long i = 0; i < numberOfCores; i++) {
        tasks[i].array = array;
        tasks[i].start = i * chunk;
        tasks[i].end = (i == numberOfCores - 1)? size: (i + 1) * chunk;
        tasks[i].value = value; // The initial value required for the array
        pthread_create(&threads[i], NULL, &threadInitArray, &tasks[i]);
    }

    // Barrier
    for (long i = 0; i < numberOfCores; i++) {
        pthread_join(threads[i], NULL);
    }

    free(threads);
    free(tasks);

}

void* threadInitArray(void* arg) {
    const InitArrayTask* task = (InitArrayTask*)arg;
    if (!task) exit(-1);
    const size_t start = task->start;
    const size_t end = task->end;
    float* array = task->array;
    const float value = task->value;

    for (size_t i = start; i < end; i++) {
        array[i] = value;
    }
    return NULL;
}

void improve(const Graph* g, thr_pool_t* pool, float* array, size_t* outlinks, const size_t size, const long numberOfCores) {

    // This array will be used to calculate the new values before storing them in the array
    float* temp = malloc(sizeof(float) * size);
    if (!temp) exit(-1);
    CalculatePageRank* pageRank = malloc(sizeof(CalculatePageRank) * size);
    if (!pageRank) exit(-1);

    // We want to zero temp
    initArray(temp, size, numberOfCores, 0);

    // We want to enqueue the tasks. We have 'size' nodes in the graph, and we want to create a task for each one
    for (size_t i = 0; i < size; i++) {
        pageRank[i].temp = temp;
        pageRank[i].array = array;
        pageRank[i].index = i;
        pageRank[i].outlinksCount = outlinks;
        pageRank[i].outlinks = g->adjacencyLists[i];
        pageRank[i].graph = g;
    }


    free(pageRank);
    free(temp);
}

void getOutlinks(const Graph* g, size_t* result, const size_t size, const long numberOfCores) {

    // We want to create a thread pool with the tasks of finding the outlinks for each vertex
    thr_pool_t* pool = thr_pool_create(numberOfCores, numberOfCores, 0, NULL);
    if (!pool) exit(-1);
    CountOutlinks* outlinkTasks = malloc(sizeof(CountOutlinks) * size);
    if (!outlinkTasks) exit(-1);


    for (size_t i = 0; i < size; i++) {
        outlinkTasks[i].array = result;
        outlinkTasks[i].index = i;
        outlinkTasks[i].outlinks = g->adjacencyLists[i];

        // Queue the task
        thr_pool_queue(pool, &threadGetOutlinks, &outlinkTasks[i]);
    }

    // We want to wait for all the tasks to complete before destroying the pool
    thr_pool_wait(pool);
    thr_pool_destroy(pool);

    free(outlinkTasks);
}

void* threadGetOutlinks(void* arg) {
    CountOutlinks* outlinkTask = arg;
    if (!outlinkTask) exit(-1);

    // Each thread will scan the list in a sequential manner
    const node* p = outlinkTask->outlinks; // The node that will scan the list
    size_t count = 0; // The amount of outlinks the
    while (p) {
        count++;
        p = p->next;
    }

    // The number of outlinks for the given vertex
    outlinkTask->array[outlinkTask->index] = count;

    return NULL;
}

void* threadCalculatePageRank(void* arg) {
    CalculatePageRank* task = arg;
    if (!task) exit(-1);

    task->temp[task->index] += (1 - D) / (float)task->graph->numVertices;

    // We know that for every neighbor of v, we need to add to their pagerank value the fact that v points to them
    node* p = task->outlinks; // We want to iterate over all the vertices v points to
    while (p) {
        const vertex neighbour = p->v;

        /*
         * We want to add to the neighbors of v the value needed to add in the pagerank formula.
         * That creates a race condition, where multiple threads working on vertices that points to the same vertex
         * Will try to update the rank of the vertex all at once.
         * In order to fix that problem we use mutexes. That way, we can assure only one thread accesses the value
         * at any time.
         */
        pthread_mutex_lock(&task->graph->num_visits_mutexes[neighbour]); // Trying to capture the mutex

        /*
         * Using the formula for computing the PageRank (see README), each vertex needs to add to the vertices it
         * points to d * pagerank(v) / outlink(v)
         */
        task->temp[neighbour] += D * task->array[task->index] / (float)task->outlinksCount[task->index];

        // Releasing the mutex
        pthread_mutex_unlock(&task->graph->num_visits_mutexes[neighbour]);

        // Moving to the next neighbour
        p = p->next;
    }


    p = task->outlinks; // We want to iterate over all the vertices v points to

    // If v has no neighbours, we need to add to all the vertices d * pagerank(v) / N
    if (!p) {
        for (size_t i = 0; i < task->graph->numVertices; i++ ) {

            // Trying to capture the mutex
            pthread_mutex_lock(&task->graph->num_visits_mutexes[i]);

            // Updating the value
            task->temp[i] += D * task->array[task->index] / (float)task->graph->numVertices;

            // Releasing the mutex
            pthread_mutex_unlock(&task->graph->num_visits_mutexes[i]);

        }
    }


    return NULL;
}