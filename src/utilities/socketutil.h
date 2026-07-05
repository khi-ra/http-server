#ifndef SOCKETUTIL_H
#define SOCKETUTIL_H

#include <arpa/inet.h>
#include <err.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

/* How long to wait for a connection in idle before shutting down. */
static const int SERVER_IDLE_TIMEOUT_MS = 5000;

/* How long to idle an active connection before closing it. */
static const int SOCKET_IDLE_TIMEOUT_S = 60;

struct accepted_socket
{
    int socket_fd;
    struct sockaddr_in address;
    bool accepted;
};

/* Create and return a file descriptor for a TCP IPv4 socket. */
int create_tcp_ipv4_socket();

/* Create and return a `struct sockaddr_in` initialised with IP and PORT. */
struct sockaddr_in create_ipv4_address(char *ip, int port);

/* Await a connection on SOCKET_FD for TIMEOUT_MS. If TIMEOUT_MS = -1, wait indefinitely.
 * If a connection is received, create and return an accepted_socket
 * struct. Otherwise, return NULL. */
struct accepted_socket accept_connection(int socket_fd, int timeout_ms);

#endif
