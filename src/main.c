#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <netinet/in.h>
#include "thread.h"
#include <sys/epoll.h>
#define MAX_EVENTS 1024

#define LISTEN_PORT 8787
#define EXPOSE_PORT 30005

void* master_thread_function(void* arg);
void* expose_data_server(void* arg);
int start_expose_data_server();

int main() {
    pthread_t master_thread;

    // Start the server that will expose the merged data on port 30005
    if(start_expose_data_server() != EXIT_SUCCESS) {
        fprintf(stderr, "Failed to start the data exposure server.\n");
        return EXIT_FAILURE;
    }

    // Creating the master thread for listening on port 8787
    if(pthread_create(&master_thread, NULL, master_thread_function, NULL) != 0) {
        perror("Failed to create the master thread");
        return EXIT_FAILURE;
    }

    // Wait for the master thread to finish
    pthread_join(master_thread, NULL);

    return EXIT_SUCCESS;
}

void* master_thread_function(void* arg) {
    int server_fd, new_socket, epoll_fd;
    struct sockaddr_in address;
    int opt = 1;
    struct epoll_event event, events[MAX_EVENTS];
    int addrlen = sizeof(address);

    setup_server_socket(&server_fd, LISTEN_PORT); // Assume this function sets up the socket

    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1 failed");
        exit(EXIT_FAILURE);
    }

    event.data.fd = server_fd;
    event.events = EPOLLIN;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) == -1) {
        perror("epoll_ctl failed");
        exit(EXIT_FAILURE);
    }

    printf("Listening for connections on port %d...\n", LISTEN_PORT);

    while (1) {
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        for (int i = 0; i < n; i++) {
            if (events[i].data.fd == server_fd) {
                new_socket = accept_connection(server_fd, &address); // Assume this function accepts the connection
                // Set up non-blocking mode for new_socket if necessary
                event.data.fd = new_socket;
                event.events = EPOLLIN | EPOLLET; // Edge Triggered mode
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_socket, &event);
            } else {
                // Data ready to be read or socket ready to be written
                spawn_thread_for_connection(events[i].data.fd); // Adjust this function for efficient task delegation
            }
        }
    }
}

int start_expose_data_server() {
    pthread_t thread_id;

    if(pthread_create(&thread_id, NULL, &expose_data_server, NULL) != 0) {
        perror("Failed to create data expose server thread");
        return EXIT_FAILURE;
    }

    pthread_detach(thread_id);
    return EXIT_SUCCESS;
}

void* expose_data_server(void* arg) {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Setting up the server socket to expose data on port 30005
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(EXPOSE_PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 10) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Exposing merged data on port %d...\n", EXPOSE_PORT);

    // Accept connections and send them merged data
    while((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))>=0) {
        char *data_to_send;
        size_t data_size;

        // Retrieve the merged data
        get_merged_data(&data_to_send, &data_size);

        // Send the data to the client
        if(send(client_socket, data_to_send, data_size, 0) < 0) {
            perror("Failed to send data");
        }

        // Close the client socket after sending data
        close(client_socket);
    }

    return NULL;
}

