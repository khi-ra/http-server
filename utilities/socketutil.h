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
    struct sockaddr_in *address;
    bool accepted;
};

/* Create and return a file descriptor for a TCP IPv4 socket. */
int create_tcp_ipv4_socket();

/* Create SOCKADDR_IN struct, initialise it with IP and PORT and return
 * a pointer to it. */
struct sockaddr_in *create_ipv4_address(char *ip, int port);

/* Accept incoming connection, create ACCEPTED_SOCKET struct and return
 * a pointer to it. */
struct accepted_socket *accept_connection(int server_socket_fd);

#endif
