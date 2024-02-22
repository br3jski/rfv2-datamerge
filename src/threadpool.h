#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <pthread.h>
#include <stdbool.h>

// Forward declaration
struct ThreadPoolTask;

// Task function type
typedef void (*TaskFunction)(void*);

// ThreadPool Task structure
typedef struct ThreadPoolTask {
    TaskFunction function;
    void* argument;
    struct ThreadPoolTask* next;
} ThreadPoolTask;

// ThreadPool structure
typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t notify;
    pthread_t *threads;
    ThreadPoolTask *tasksHead;
    ThreadPoolTask *tasksTail;
    int threadCount;
    int taskQueueSize;
    int count;
    int shutdown;
    int started;
} ThreadPool;

ThreadPool* create_thread_pool(int threadCount);
int add_task_to_pool(ThreadPool* pool, TaskFunction function, void* argument);
int shutdown_pool(ThreadPool* pool, int gracefull);
int destroy_pool(ThreadPool* pool);
void* thread_pool_thread(void* threadpool);

#endif // THREADPOOL_H
