// Assume "threadpool.h" provides create_thread_pool, shutdown_pool, and add_task_to_pool functions.
#include "threadpool.h"
#include "data.h"
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

ThreadPool pool;  // Global thread pool

void init_thread_pool(size_t pool_size) {
    pool = create_thread_pool(pool_size);
}

void shutdown_thread_pool() {
    shutdown_pool(&pool);
}

void process_connection(int client_sock) {
    char buffer[1024];
    ssize_t read_size;

    while ((read_size = recv(client_sock, buffer, sizeof(buffer), 0)) > 0) {
        process_data(buffer, read_size);
    }

    if (read_size == -1 && (errno != EAGAIN && errno != EWOULDBLOCK)) {
        perror("recv");
    }

    close(client_sock);
}

void enqueue_task(int client_sock) {
    add_task_to_pool(&pool, process_connection, (void*)(size_t)client_sock);
}
