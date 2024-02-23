#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <pthread.h>

// Remove the forward declaration, it's not necessary if you're defining the structure below
// struct ThreadPoolTask; // Removed

// Task function type
typedef void (*TaskFunction)(void*);

// ThreadPool Task structure
typedef struct ThreadPoolTask {
    TaskFunction function;
    void* argument;
    struct ThreadPoolTask* next;
} ThreadPoolTask;

// ThreadPool structure
typedef struct ThreadPool {
    pthread_mutex_t lock;
    pthread_cond_t notify;
    pthread_t *threads;
    ThreadPoolTask *tasksHead, *tasksTail;
    int threadCount;
    int taskQueueSize;
    int count;
    int shutdown;
    int started;
} ThreadPool;

ThreadPool* create_thread_pool(int threadCount);
int add_task_to_pool(ThreadPool* pool, TaskFunction function, void* argument);

// Ensure the shutdown_pool declaration matches your implementation.
// If you're not using the 'gracefull' parameter in the implementation, consider removing it from here
// For this correction, assuming 'gracefull' is not used:
int shutdown_pool(ThreadPool* pool); // Removed 'int gracefull' parameter

int destroy_pool(ThreadPool* pool);

// If thread_pool_thread function is intended only for internal use within threadpool.c,
// and not to be exposed or used directly by other parts of your program,
// it's better to not declare it in the header file. So, consider removing this line:
// void* thread_pool_thread(void* threadpool); // Consider removing this line if it's for internal use

#endif // THREADPOOL_H
