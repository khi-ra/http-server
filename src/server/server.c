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

struct connection
{
    int event_fd;
    struct accepted_socket client_socket;
};

static int create_detached_thread(void *subroutine, void *subroutine_arg);
static void *thread_handle_connection(void *args);
static struct connection *create_thread_data(int efd, struct accepted_socket client_socket);

int receive_msg(struct accepted_socket *accepted_socket, char *buffer);
void write_msg(struct accepted_socket *accepted_socket, char *buffer);

/* Create and detach a thread, assigning it the execution of SUBROUTINE
 * with argument SUBROUTINE_ARG.
 *
 * Return 0 on success or a non-zero error number on error.
 */
static int create_detached_thread(void *subroutine, void *subroutine_arg)
{
    pthread_t thread_id;
    int errnum;

    errnum = pthread_create(&thread_id, NULL, subroutine, subroutine_arg);
    if (errnum != 0)
        return -1;

    errnum = pthread_detach(thread_id);
    if (errnum != 0)
        return -1;

    return errnum;
}

static struct connection *create_thread_data(int efd, struct accepted_socket client_socket)
{
    struct connection *t_data = malloc(sizeof(struct connection));
    t_data->event_fd = efd;
    t_data->client_socket = client_socket;

    return t_data;
}

/* Receive messages from connecting peer and write them to stdout.
 * To be assigned to a thread upon creation. */
static void *thread_handle_connection(void *args)
{
    struct connection *connection_data = args;

    while (true)
    {
        char buffer[BUFFSIZE + 1];
        int n_recv = receive_msg(&connection_data->client_socket, buffer);

        if (n_recv == -1)
        {
            if (errno == EWOULDBLOCK || errno == EAGAIN)
                printf("Client idle timeout, closing connection...\n");
            else
                error_handler(errno, "receive failed");

            break;
        }
        if (n_recv == 0)
            break;

        write_msg(&connection_data->client_socket, buffer);
    }

    printf("[thread id: %lu] Shutting down...\n", pthread_self());
    close(connection_data->client_socket.socket_fd);
    pthread_exit(NULL);
}

/* Read message received on SOCK_FD. Return number of bytes received
 * or -1 for error. */
int receive_msg(struct accepted_socket *accepted_socket, char *buffer)
{
    int n_recv = recv(accepted_socket->socket_fd, buffer, BUFFSIZE, 0);

    if (n_recv > 0)
        buffer[n_recv] = 0;

    return n_recv;
}

/* Write message in BUFFER to stdout in the format "IP:PORT = BUFFER". */
void write_msg(struct accepted_socket *accepted_socket, char *buffer)
{
    char ip[INET_ADDRSTRLEN];
    short port;

    // convert ip and port to human readable format
    inet_ntop(accepted_socket->address.sin_family, &accepted_socket->address.sin_addr.s_addr, ip, sizeof(ip));
    port = ntohs(accepted_socket->address.sin_port);

    printf("Message from %s:%hu = %s \n", ip, port, buffer);
}

int main()
{
    int event_fd;
    int socket_fd;
    struct sockaddr_in address;
    struct accepted_socket client_socket;
    int setup_successful = 1;

    // to enable communication between the main thread and worker threads
    event_fd = eventfd(0, EFD_NONBLOCK);
    if (event_fd == -1)
    {
        error_handler(errno, "creating events file failed");
        setup_successful = 0;
    }

    // create server endpoint of TCP socket
    address = create_ipv4_address("", 8080);
    if ((socket_fd = create_tcp_ipv4_socket()) == -1)
    {
        error_handler(errno, "creating socket failed");
        setup_successful = 0;
    }
    else if (bind(socket_fd, (struct sockaddr *) &address, sizeof(struct sockaddr)) == -1)
    {
        error_handler(errno, "bind failed");
        setup_successful = 0;
    }

    if (setup_successful)
        printf("Server socket successfully created\n");

    if ((listen(socket_fd, MAXCONN)) == -1)
    {
        error_handler(errno, "listen failed");
        setup_successful = 0;
    }

    // create a pollfd struct for the socket file and events file
    struct pollfd polled_files[2] = {
        {.fd = socket_fd, .events = POLLIN},
        {.fd = event_fd, .events = POLLIN},
    };
    int polled_files_len = sizeof(polled_files) / sizeof(polled_files[0]);

    int count_connections = 0;
    int poll_time_ms;
    while (setup_successful)
    {
        // poll socket and event file
        bool connection_received = false;
        bool server_timed_out = false;
        while (!connection_received && !server_timed_out)
        {
            // the server shouldn't idle on 0 connections indefinitely
            if (count_connections > 0)
                poll_time_ms = INDEFINITE;
            else
                poll_time_ms = SERVER_IDLE_TIMEOUT_MS;

            int poll_result = poll(polled_files, polled_files_len, poll_time_ms);

            // exit program if server timed out
            if (poll_result == 0)
                server_timed_out = true;
            else if (poll_result == -1)
            {
                error_handler(errno, "poll failed");
                server_timed_out = true; // to break loop and end program
            }
            else // loop through poll_files array to check which file th event was received on
            {
                struct pollfd file; // placeholder to help with readability
                for (int i = 0; i < polled_files_len; i++)
                {
                    file = polled_files[i];
                    bool event_received = file.revents & POLLIN;

                    if (event_received)
                    {
                        if (file.fd == socket_fd)
                            connection_received = true;
                        else if (file.fd == event_fd)
                        {
                            // read counter from events file and decrement count_connections
                            uint64_t count_closed;
                            if (read(event_fd, &count_closed, sizeof(count_closed)) == -1)
                            {
                                error_handler(errno, "read from events file failed");
                                server_timed_out = true; // to break loop and end program
                            }
                            else
                                count_connections -= count_closed;
                        }
                    }
                }
            }
        }

        if (server_timed_out)
            break;

        if (connection_received)
            client_socket = accept_connection(socket_fd);

        if (client_socket.accepted)
        {
            struct connection *t_data = create_thread_data(event_fd, client_socket);
            int errnum = create_detached_thread(thread_handle_connection, t_data);
            if (errnum != 0)
            {
                error_handler(errnum, "creating thread failed");
                free(t_data);
            }
            else
            {
                printf("Connection successfully received\n");
                count_connections++;
            }
        }
    }
    printf("No active connections, closing server\n");

    // cleanup
    close(socket_fd);
    close(event_fd);
    return 0;
}
