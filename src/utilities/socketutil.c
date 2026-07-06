#include "socketutil.h"

int poll_read_events(int socket_fd, int duration_ms);

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

struct accepted_socket accept_connection(int socket_fd, int timeout_ms)
{
    struct accepted_socket accepted_socket;
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);

    if (poll_read_events(socket_fd, timeout_ms) <= 0)
    {
        accepted_socket.accepted = false;
        return accepted_socket;
    }

    // accept connection
    int fd = accept(socket_fd, (struct sockaddr *) &addr, &addr_size);
    accepted_socket.socket_fd = fd;
    accepted_socket.address = addr;
    accepted_socket.accepted = accepted_socket.socket_fd > 0;

    return accepted_socket;
}

/* Poll for read events on SOCKET_FD for DURATION_MS.
 * Upon success, return the number of file descriptors with events.
 * Otherwise, return 0 if timed out or -1 for error. */
int poll_read_events(int socket_fd, int duration_ms)
{
    // initialise poll request struct for read events
    struct pollfd pfd;
    pfd.fd = socket_fd;
    pfd.events = POLLIN;

    // wait duration_ms milliseconds for read events
    int poll_fds = poll(&pfd, 1, duration_ms);
    return poll_fds;
}
