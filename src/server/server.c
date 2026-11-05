#include "../utilities/errorutil.h"
#include "../utilities/socketutil.h"
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#define INDEFINITE -1 // to use for polling duration
#define MAXCONN 5
#define BUFFSIZE 1024

static int errnum;

struct connection
{
    socket_t client;
    int server_eventfd;
};

static int setup(socket_t *server, char *ip, int port);
socket_t accept_connection(int socket_fd);

int receive_msg(int sock_fd, char *buffer);
void write_msg(socket_t *client, char *buffer);

/* Initialise *SERVER with IP and PORT, and mark the server as a passive socket
 * with `listen()`.
 *
 * Return 0 on success or -1 on error.
 */

int setup(socket_t *server, char *ip, int port)
{
    struct sockaddr_in address = create_ipv4_address(ip, port);
    int socket_fd = create_tcp_ipv4_socket();
    if (socket_fd < 0)
        goto error_out;

    int result;
    result = bind(socket_fd, (struct sockaddr *) &address, sizeof(struct sockaddr));
    if (result < 0)
        goto error_out;

    result = listen(socket_fd, MAXCONN);
    if (result < 0)
        goto error_out;

    server->fd = socket_fd;
    server->address = address;
    return 0;

error_out:
    errnum = ERR_SETUP;
    return -1;
}

/* Accept a connection on SOCKET_FD.
 *
 * On success, return a `client_socket_t` struct initialised with the peer's socket data
 * and a valid file descriptor for `client_socket_t.fd`. On error or timeout, return `client_socket_t`
 * with `.socket_fd` set to -1.
 */
socket_t accept_connection(int socket_fd)
{
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(addr);

    int fd = accept(socket_fd, (struct sockaddr *) &addr, &addr_size);
    if (fd < 0)
        errnum = ERR_ACCEPT;

    socket_t client;
    client.fd = fd;
    client.address = addr;

    return client;
}

/* Receive messages from client and write them to stdout.
 * Close the client's file descriptor once connection ends.
 */
static int handle_conn(socket_t *client)
{
    char buffer[BUFFSIZE];
    while (true)
    {
        int n_recv = receive_msg(client->fd, buffer);
        if (n_recv == 0)
            return 0;
        if (n_recv < 0)
        {
            errnum = ERR_SOCK_IO;
            return -1;
        }

        buffer[n_recv] = '\0';
        write_msg(client, buffer);
    }
}

/* Read message received on SOCK_FD. Return number of bytes received or -1
 * for error.
 */
int receive_msg(int sock_fd, char *buffer)
{
    int n_recv = recv(sock_fd, buffer, BUFFSIZE, 0);

    if (n_recv > 0)
        buffer[n_recv] = 0;
    else if (n_recv < 0)
        errnum = ERR_SOCK_IO;

    return n_recv;
}

/* Write the message in BUFFER to stdout. */
void write_msg(socket_t *socket, char *buffer)
{
    char ip[INET_ADDRSTRLEN];
    short port;

    // convert ip and port to human readable format
    inet_ntop(socket->address.sin_family, &socket->address.sin_addr.s_addr, ip, sizeof(ip));
    port = ntohs(socket->address.sin_port);

    printf("Message from %s:%hu = %s \n", ip, port, buffer);
}

int main()
{
    socket_t server;

    int setup_result = setup(&server, "", 8080);
    if (setup_result < 0)
        goto error_out;

    printf("Server socket successfully created\n");

    struct pollfd server_socket_pfd = {.fd = server.fd, .events = POLLIN};

    while (true)
    {
        int poll_res = poll(&server_socket_pfd, 1, INDEFINITE);
        if (poll_res < 0)
            goto error_out;

        // no incoming events
        if (!(server_socket_pfd.revents & POLLIN))
            continue;

        socket_t client = accept_connection(server.fd);
        if (client.fd < 0)
            goto error_out;

        printf("Connection successfully received\n");
        int conn_res = handle_conn(&client);
        if (conn_res < 0)
            error_handler(errno, err_str(errnum));
    }

error_out:
    error_handler(errno, err_str(errnum));
    printf("Closing server...\n");
    close(server.fd);
}
