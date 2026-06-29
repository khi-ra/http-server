#include "socketutil.h"
#include <netinet/in.h>

int create_tcp_ipv4_socket()
{
    return socket(AF_INET, SOCK_STREAM, 0);
}

struct sockaddr_in *create_ipv4_address(char *ip, int port)
{
    struct sockaddr_in *address = malloc(sizeof(struct sockaddr_in));

    address->sin_family = AF_INET;

    if (port)
        address->sin_port = htons(port);

    if (strlen(ip) == 0) // listen for any ip address if none is specified
        address->sin_addr.s_addr = INADDR_ANY;
    else
        inet_pton(AF_INET, ip, &address->sin_addr.s_addr);

    return address;
}

struct accepted_socket *accept_connection(int server_socket_fd)
{
    struct accepted_socket *accepted_socket = malloc(sizeof(struct accepted_socket));
    accepted_socket->address = NULL;
    socklen_t address_size = sizeof(struct sockaddr_in);

    // accept incoming connection and populate struct
    accepted_socket->socket_fd = accept(server_socket_fd, (struct sockaddr *) accepted_socket->address, &address_size);
    accepted_socket->accepted = accepted_socket->socket_fd > 0;

    return accepted_socket;
}
