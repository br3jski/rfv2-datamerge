#include "thread.h"
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include "data.h" // Assume this includes the process_data function prototype
#include <errno.h> 


// Utility function to set a socket to non-blocking mode
int make_socket_non_blocking(int sfd) {
    int flags = fcntl(sfd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl F_GETFL");
        return -1;
    }

    flags |= O_NONBLOCK;
    if (fcntl(sfd, F_SETFL, flags) == -1) {
        perror("fcntl F_SETFL");
        return -1;
    }

    return 0;
}

// Thread function for processing data from a connection
void* connection_handler(void* arg) {
    int sock = *(int*)arg;
    free(arg); // Clean up the heap memory allocated for the socket descriptor

    char buffer[1024]; // Adjust size as needed for your application
    ssize_t read_size;

    // Loop to read data as long as the socket has data available
    while ((read_size = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
        process_data(buffer, read_size); // Process the received data
    }

    if (read_size == -1 && (errno != EAGAIN || errno != EWOULDBLOCK)) {
        perror("recv");
    }

    close(sock); // Close the connection
    return NULL;
}

// Function to spawn a new thread for a connection
void spawn_thread_for_connection(int client_sock) {
    pthread_t thread;
    int* new_sock_ptr = malloc(sizeof(int));
    if (!new_sock_ptr) {
        perror("Failed to allocate memory for socket descriptor");
        return;
    }
    *new_sock_ptr = client_sock;

    if (make_socket_non_blocking(client_sock) == -1) // Make the socket non-blocking
        exit(EXIT_FAILURE);

    if (pthread_create(&thread, NULL, connection_handler, new_sock_ptr) != 0) {
        perror("Failed to create thread");
        free(new_sock_ptr); // Clean up memory if thread creation fails
    }

    pthread_detach(thread); // The thread resources are automatically freed upon termination
}
