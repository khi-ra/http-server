#include "../utilities/errorutil.h"
#include "../utilities/socketutil.h"
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/eventfd.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#define INDEFINITE -1 // to use for polling duration
#define MAXCONN 5
#define BUFFSIZE 1024

/* How long to wait for a connection in idle before shutting down. */
static const int SERVER_IDLE_TIMEOUT_MS = 5000;
static int errnum;

struct server
{
    int socket_fd;
    int event_fd;
    struct sockaddr_in address;
};

struct accepted_socket
{
    int fd;
    struct sockaddr_in address;
    bool accepted;
};

struct connection
{
    struct server server;
    struct accepted_socket client_socket;
};

static int setup(struct server *server, char *ip, int port);
struct accepted_socket accept_connection(int socket_fd);

static int create_detached_thread(void *subroutine, void *subroutine_arg);
static void *thread_handle_connection(void *args);
static struct connection *create_thread_data(struct server *server, struct accepted_socket *client_socket);

int receive_msg(struct accepted_socket *accepted_socket, char *buffer);
void write_msg(struct accepted_socket *accepted_socket, char *buffer);

/* Initialise *SERVER with IP and PORT, and mark the server as a passive socket
 * with `listen()`.
 *
 * Return 0 on success, -1 on error.
 */
int setup(struct server *server, char *ip, int port)
{
    int event_fd;
    int socket_fd;
    int err_flag = -1;

    event_fd = eventfd(0, EFD_NONBLOCK);
    if (event_fd == -1)
        goto out;

    struct sockaddr_in address = create_ipv4_address(ip, port);
    socket_fd = create_tcp_ipv4_socket();
    if (socket_fd == -1)
        goto out;

    int bind_result = bind(socket_fd, (struct sockaddr *) &address, sizeof(struct sockaddr));
    if (bind_result == -1)
        goto out;

    int listen_result = listen(socket_fd, MAXCONN);
    if (listen_result == -1)
        goto out;

    err_flag = 0;
    server->socket_fd = socket_fd;
    server->event_fd = event_fd;
    server->address = address;

out:
    if (err_flag == -1)
        errnum = ERR_SETUP;
    return err_flag;
}

/* Accept a connection on SOCKET_FD.
 *
 * On success, return a `accepted_socket` initialised with the peer's socket data
 * and `.accepted` set to true.
 *
 * On error or timeout, return `accepted_socket` with `.socket_fd` set to -1 and
 * `.accepted` set to false.
 */
struct accepted_socket accept_connection(int socket_fd)
{
    struct accepted_socket accepted_socket;
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);

    int fd = accept(socket_fd, (struct sockaddr *) &addr, &addr_size);
    if (fd == -1)
        errnum = ERR_ACCEPT;
    accepted_socket.fd = fd;
    accepted_socket.address = addr;
    accepted_socket.accepted = accepted_socket.fd > 0;

    return accepted_socket;
}

/* Create and detach a thread, assigning it the execution of SUBROUTINE with
 * argument SUBROUTINE_ARG.
 *
 * Return 0 on success or a non-zero error number on error.
 */
static int create_detached_thread(void *subroutine, void *subroutine_arg)
{
    pthread_t thread_id;
    int errnum = 0;

    errnum = pthread_create(&thread_id, NULL, subroutine, subroutine_arg);
    if (errnum != 0)
        goto out;

    errnum = pthread_detach(thread_id);
    if (errnum != 0)
        goto out;

out:
    return errnum;
}

static struct connection *create_thread_data(struct server *server, struct accepted_socket *client_socket)
{
    struct connection *t_data = malloc(sizeof(struct connection));
    t_data->server = *server;
    t_data->client_socket = *client_socket;

    return t_data;
}

/* Receive messages from connecting peer and write them to stdout.
 * To be assigned to a thread upon creation.
 */
static void *thread_handle_connection(void *args)
{
    struct connection *connection_data = args;
    int err_code = -1;

    while (true)
    {
        char buffer[BUFFSIZE + 1];
        int n_recv = receive_msg(&connection_data->client_socket, buffer);

        if (n_recv == -1)
        {
            bool socket_timed_out = (errno == EWOULDBLOCK || errno == EAGAIN);
            // socket timeout is not considered an error
            if (!socket_timed_out)
                errnum = ERR_SOCK_IO;

            break;
        }
        if (n_recv == 0)
            break;

        write_msg(&connection_data->client_socket, buffer);
    }

    uint64_t exit_signal = 1;
    if (write(connection_data->server.event_fd, &exit_signal, sizeof(exit_signal)) == -1)
    {
        // only set err_code if it wasn't set earlier
        if (err_code == -1)
            err_code = ERR_EVENTFD;
    }

    if (errnum != -1)
        error_handler(errno, err_str(errnum));

    printf("[thread id: %lu] Shutting down...\n", pthread_self());
    close(connection_data->client_socket.fd);
    free(connection_data);
    pthread_exit(NULL);
}

/* Read message received on SOCK_FD. Return number of bytes received or -1
 * for error.
 */
int receive_msg(struct accepted_socket *socket, char *buffer)
{
    int n_recv = recv(socket->fd, buffer, BUFFSIZE, 0);

    if (n_recv > 0)
        buffer[n_recv] = 0;
    else if (n_recv == -1)
        errnum = ERR_SOCK_IO;

    return n_recv;
}

/* Write the message in BUFFER to stdout. */
void write_msg(struct accepted_socket *socket, char *buffer)
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
    struct server server;
    struct connection *connection = NULL;

    int setup_result = setup(&server, "", 8080);
    if (setup_result == -1)
        goto error_out;

    printf("Server socket successfully created\n");

    // create a pollfd struct for the socket file and events file
    struct pollfd polled_files[2] = {
        {.fd = server.socket_fd, .events = POLLIN},
        {.fd = server.event_fd, .events = POLLIN},
    };
    int polled_files_len = sizeof(polled_files) / sizeof(polled_files[0]);

    int count_connections = 0;
    while (true)
    {
        // poll socket and event file
        bool connection_received = false;
        while (!connection_received)
        {
            // prevent the server indefinitely idling on 0 connections
            int poll_time_ms;
            if (count_connections > 0)
                poll_time_ms = INDEFINITE;
            else
                poll_time_ms = SERVER_IDLE_TIMEOUT_MS;

            int poll_result = poll(polled_files, polled_files_len, poll_time_ms);

            // server timed out
            if (poll_result == 0)
                goto out;

            if (poll_result == -1)
            {
                errnum = ERR_POLL;
                goto error_out;
            }

            struct pollfd file; // placeholder to help with readability
            for (int i = 0; i < polled_files_len; i++)
            {
                file = polled_files[i];
                bool event_on_file = file.revents & POLLIN;

                if (event_on_file)
                {
                    if (file.fd == server.socket_fd)
                        connection_received = true;
                    else if (file.fd == server.event_fd)
                    {
                        int64_t count_closed;
                        if (read(server.event_fd, &count_closed, sizeof(count_closed)) == -1)
                        {
                            errnum = ERR_EVENTFD;
                            goto error_out;
                        }
                        count_connections -= count_closed;
                    }
                }
            }
        }

        struct accepted_socket client_socket = accept_connection(server.socket_fd);
        if (client_socket.accepted)
        {
            struct connection *t_data = create_thread_data(&server, &client_socket);

            errno = create_detached_thread(thread_handle_connection, t_data);
            if (errno != 0)
                goto error_free_conn;

            printf("Connection successfully received\n");
            count_connections++;
        }
    }

error_free_conn:
    free(connection);
error_out:
    error_handler(errno, err_str(errnum));
out:
    printf("No active connections, closing server...\n");
    close(server.socket_fd);
    close(server.event_fd);
}
