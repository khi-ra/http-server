#include "socketutil.h"

int create_tcp_ipv4_socket()
{
    return socket(AF_INET, SOCK_STREAM, 0);
}

void create_ipv4_address(struct sockaddr_in *address, char *ip, int port)
{
    address = realloc(address, sizeof(struct sockaddr_in));

    address->sin_family = AF_INET;
    address->sin_port = htons(port);

    if (strlen(ip) == 0) // listen for any ip address if none is specified
        address->sin_addr.s_addr = INADDR_ANY;
    else
        inet_pton(AF_INET, ip, &address->sin_addr.s_addr);
}
