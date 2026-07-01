#ifndef SOCKETUTIL_H
#define SOCKETUTIL_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

/* How long to wait for a connection before returning. */
static const int SOCKET_TIMEOUT_MS = 5000;

struct accepted_socket
{
    int socket_fd;
    struct sockaddr_in address;
    bool accepted;
};

/* Create and return a file descriptor for a TCP IPv4 socket. */
int create_tcp_ipv4_socket();

/* Initialise *ADDR with IP and PORT. */
void create_ipv4_address(struct sockaddr_in **addr, char *ip, int port);

/* Await a connection on SOCKET_FD for TIMEOUT_MS. If TIMEOUT_MS = -1, wait indefinitely.
 * If a connection is received, create and return a pointer to struct accepted_socket.
 * Otherwise, return NULL. */
struct accepted_socket *accept_connection(int socket_fd, int timeout_ms);

/* Poll for read events on SOCKET_FD for DURATION_MS.
 * Return the number of file descriptors with events, 0 if timed out or -1 for error. */
int poll_read_event(int socket_fd, int duration_ms);

#endif
