#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include "threadpool.h"

#define LISTEN_PORT 8787
#define MAX_EVENTS 1024

ThreadPool* pool;

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

void handle_connection(void* arg) {
    int client_sock = (int)(size_t)arg;
    char buffer[1024]; // Buffer for storing received data
    ssize_t bytes_read;

    // Attempt to read data from the client
    bytes_read = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
    if (bytes_read < 0) {
        perror("recv");
        close(client_sock);
        return;
    }

    // Null-terminate the received data and process it
    buffer[bytes_read] = '\0';
    printf("Received: %s\n", buffer);

    // Example processing: simply echo back the received data for demonstration
    // In a real application, you might process the data and generate a different response
    if (send(client_sock, buffer, bytes_read, 0) < 0) {
        perror("send");
        close(client_sock);
        return;
    }

    // Close the socket when done
    close(client_sock);
}

int main() {
    int server_fd, epoll_fd;
    struct sockaddr_in address;
    struct epoll_event event, events[MAX_EVENTS];

    pool = create_thread_pool(4); // Adjust the number of threads based on your needs

    // Standard setup for server socket, including bind and listen
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    // Error checks omitted for brevity
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(LISTEN_PORT);
    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 10);

    make_socket_non_blocking(server_fd);

    epoll_fd = epoll_create1(0);
    event.data.fd = server_fd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event);

    while (1) {
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        for (int i = 0; i < n; i++) {
            if (events[i].data.fd == server_fd) {
                int client_sock;
                while ((client_sock = accept(server_fd, NULL, NULL)) > 0) {
                    make_socket_non_blocking(client_sock);
                    event.data.fd = client_sock;
                    event.events = EPOLLIN | EPOLLET;
                    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_sock, &event);
                }
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    perror("accept");
                }
            } else {
                add_task_to_pool(pool, handle_connection, (void*)(size_t)events[i].data.fd);
            }
        }
    }

    // Shutdown and cleanup omitted for brevity
    return 0;
}
