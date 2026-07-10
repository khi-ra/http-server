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
#include <unistd.h>

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

/* Accept a connection on SOCKET_FD.
 *
 * On success, return `accepted_socket` with a valid fd, `.accepted` set to true,
 * and `.address` initialised to the connected peer's address.
 *
 * On error or timeout, return `accepted_socket` with `.accepted` set to false. */
struct accepted_socket accept_connection(int socket_fd);

#endif
