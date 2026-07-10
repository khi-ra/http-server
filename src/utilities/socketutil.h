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

/* How long to idle an active connection before closing it. */
static const int SOCKET_IDLE_TIMEOUT_S = 60;

/* Create and return a file descriptor for a TCP IPv4 socket. */
int create_tcp_ipv4_socket();

/* Create and return a `struct sockaddr_in` initialised with IP and PORT. */
struct sockaddr_in create_ipv4_address(char *ip, int port);

#endif
