#include "socketutil.h"
#include <err.h>
#include <poll.h>
#include <stdlib.h>
#include <netinet/in.h>

int create_tcp_ipv4_socket()
{
    return socket(AF_INET, SOCK_STREAM, 0);
}

void create_ipv4_address(struct sockaddr_in **addr, char *ip, int port)
{
    *addr = realloc(*addr, sizeof(struct sockaddr_in));

    (*addr)->sin_family = AF_INET;

    if (port)
        (*addr)->sin_port = htons(port);

    if (strlen(ip) == 0) // listen for any ip address if none is specified
        (*addr)->sin_addr.s_addr = INADDR_ANY;
    else
        inet_pton(AF_INET, ip, &(*addr)->sin_addr.s_addr);
}

struct accepted_socket *accept_connection(int socket_fd)
{
    // initialise poll file descriptor struct for readability events
    struct pollfd pfd;
    pfd.fd = socket_fd;
    pfd.events = POLLIN;

    // wait for SOCKET_TIMEOUT ms for readability events
    int poll_ret = poll(&pfd, 1, SOCKET_TIMEOUT);

    if (poll_ret == 0) // no read events return NULL
	return NULL;
    else if (poll_ret < 0) // poll errored out
    {
	fprintf(stderr, "[POLL ERROR] : %i", poll_ret);
	return NULL;
    }

    // accept connection
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    int fd = accept(socket_fd, (struct sockaddr *) &addr, &addr_size);

    // create and populate struct
    struct accepted_socket *accepted_socket = malloc(sizeof(struct accepted_socket));

    accepted_socket->socket_fd = fd;
    accepted_socket->address = addr;
    accepted_socket->accepted = accepted_socket->socket_fd > 0;

    return accepted_socket;
}
