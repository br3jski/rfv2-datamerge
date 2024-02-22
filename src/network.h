#ifndef NETWORK_H
#define NETWORK_H

#include <netinet/in.h>

void setup_server_socket(int *server_fd, int port);
void accept_connection(int server_fd, int *client_socket, struct sockaddr_in *address);

#endif // NETWORK_H
