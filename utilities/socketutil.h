#ifndef SOCKETUTIL_H
#define SOCKETUTIL_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

struct accepted_socket
{
    int socket_fd;
    struct sockaddr_in address;
    bool accepted;
};

/* Create and return a file descriptor for a TCP IPv4 socket. */
int create_tcp_ipv4_socket();

/* Initialise *ADDRESS with IP and PORT. */
void create_ipv4_address(struct sockaddr_in **address, char *ip, int port);

/* Accept connection on SOCKET_FD and create and return *ACCEPTED_SOCKET. */
struct accepted_socket *accept_connection(int socket_fd);

#endif
