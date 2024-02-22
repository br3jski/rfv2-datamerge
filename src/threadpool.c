#include "threadpool.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct ThreadPoolTask {
    TaskFunction function;
    void* argument;
    struct ThreadPoolTask* next;
} ThreadPoolTask;

struct ThreadPool {
    pthread_mutex_t lock;
    pthread_cond_t notify;
    pthread_t *threads;
    ThreadPoolTask *tasksHead, *tasksTail;
    int threadCount, taskQueueSize, count, shutdown, started;
};

static void* thread_pool_thread(void* threadpool);

ThreadPool* create_thread_pool(int threadCount) {
    ThreadPool* pool = (ThreadPool*)malloc(sizeof(ThreadPool));
    if (!pool) goto err;

    // Initialize
    pool->threadCount = threadCount;
    pool->count = pool->shutdown = pool->started = 0;
    pool->tasksHead = pool->tasksTail = NULL;
    pool->threads = (pthread_t*)malloc(sizeof(pthread_t) * threadCount);

    if (!pool->threads) goto err;

    if (pthread_mutex_init(&(pool->lock), NULL) != 0 ||
        pthread_cond_init(&(pool->notify), NULL) != 0) goto err;

    for (int i = 0; i < threadCount; ++i) {
        if (pthread_create(&(pool->threads[i]), NULL, thread_pool_thread, (void*)pool) != 0) {
            destroy_pool(pool);
            return NULL;
        }
        pool->started++;
    }

    return pool;

err:
    if (pool) {
        if (pool->threads) free(pool->threads);
        free(pool);
    }
    return NULL;
}

int add_task_to_pool(ThreadPool* pool, TaskFunction function, void* argument) {
    if (!pool || !function) return -1;

    ThreadPoolTask* task = (ThreadPoolTask*)malloc(sizeof(ThreadPoolTask));
    if (!task) return -1;

    task->function = function;
    task->argument = argument;
    task->next = NULL;

    pthread_mutex_lock(&(pool->lock));
    
    if (pool->tasksHead == NULL) {
        pool->tasksHead = task;
    } else {
        pool->tasksTail->next = task;
    }
    pool->tasksTail = task;
    pool->count++;

    pthread_cond_signal(&(pool->notify));
    pthread_mutex_unlock(&(pool->lock));

    return 0;
}

static void* thread_pool_thread(void* threadpool) {
    ThreadPool* pool = (ThreadPool*)threadpool;

    while (1) {
        pthread_mutex_lock(&(pool->lock));

        while ((pool->count == 0) && (!pool->shutdown)) {
            pthread_cond_wait(&(pool->notify), &(pool->lock));
        }

        if (pool->shutdown) {
            break;
        }

        ThreadPoolTask* task = pool->tasksHead;
        if (task) {
            pool->tasksHead = task->next;
            pool->count--;
            if (pool->tasksHead == NULL) {
                pool->tasksTail = NULL;
            }
        }

        pthread_mutex_unlock(&(pool->lock));

        if (task) {
            task->function(task->argument);
            free(task);
        }
    }

    pool->started--;
    pthread_mutex_unlock(&(pool->lock));
    pthread_exit(NULL);
    return (NULL);
}

int shutdown_pool(ThreadPool* pool) {
    if (!pool || pool->shutdown) return -1;

    pthread_mutex_lock(&(pool->lock));
    pool->shutdown = 1;
    pthread_cond_broadcast(&(pool->notify));
    pthread_mutex_unlock(&(pool->lock));

    for (int i = 0; i < pool->threadCount; i++) {
        if (pthread_join(pool->threads[i], NULL) != 0) {
            return -1;
        }
    }
    return 0;
}

int destroy_pool(ThreadPool* pool) {
    if (!pool || pool->shutdown) {
        return -1;
    }

    pool->shutdown = 1;

    // Wake up all worker threads
    pthread_cond_broadcast(&(pool->notify));

    // Join all worker thread
    for (int i = 0; i < pool->threadCount; i++) {
        pthread_join(pool->threads[i], NULL);
    }

    // Cleanup
    ThreadPoolTask *current;
    while (pool->tasksHead != NULL) {
        current = pool->tasksHead;
        pool->tasksHead = current->next;
        free(current);
    }

    pthread_mutex_destroy(&(pool->lock));
    pthread_cond_destroy(&(pool->notify));
    free(pool->threads);
    free(pool);

    return 0;
}
