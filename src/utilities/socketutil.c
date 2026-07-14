#include "socketutil.h"

int create_tcp_ipv4_socket()
{
    struct timeval timeout;
    timeout.tv_sec = SOCKET_IDLE_TIMEOUT_S;
    timeout.tv_usec = 0;
    int socket_fd;

    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        return -1;

    // avoid leaking socket_fd if socket() succeeds but setsockopt() fails
    if (setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1)
    {
        close(socket_fd);
        return -1;
    }

    return socket_fd;
}

struct sockaddr_in create_ipv4_address(char *ip, int port)
{
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;

    if (port)
        addr.sin_port = htons(port);

    if (strlen(ip) == 0) // listen for any ip address if none is specified
        addr.sin_addr.s_addr = INADDR_ANY;
    else
        inet_pton(AF_INET, ip, &addr.sin_addr.s_addr);

    return addr;
}
