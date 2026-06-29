#ifndef SOCKETUTIL_H
#define SOCKETUTIL_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

/* Create and return a file descriptor for a TCP IPv4 socket. */
int create_tcp_ipv4_socket();

/* Create SOCKADDR_IN struct, initialise it with IP and PORT
 * and return a pointer to it. */
struct sockaddr_in *create_ipv4_address(char *ip, int port);

#endif
