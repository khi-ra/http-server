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

/* Dynamically allocate *ADDRESS and initialise it with IP and PORT. */
void create_ipv4_addr(struct sockaddr_in *address, char *ip, int port);

#endif
