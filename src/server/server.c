#include "../utilities/errorutil.h"
#include "../utilities/socketutil.h"
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAXCONN 5
#define BUFFSIZE 1024

static int n_connections = 0;

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

int main()
{
    int socket_fd;
    struct sockaddr_in address;
    struct accepted_socket accepted_socket;
    struct sigaction sigchld_action;
    int setup_successful = 1;

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

    // accept connections and handle connected client's requests
    while (setup_successful)
    {
        // set the polling duration based on the number of active connections
        int timeout_ms = 0;
        if (n_connections == 0)
            timeout_ms = SERVER_IDLE_TIMEOUT_MS;
        else if (n_connections > 0)
            timeout_ms = -1;

        accepted_socket = accept_connection(socket_fd, timeout_ms);

        if (!accepted_socket.accepted && timeout_ms > 0)
            break;

        if (accepted_socket.accepted)
        {
            printf("Connection successfully received\n");

            pid_t handler_pid = fork();
            if (handler_pid == 0)
            {
                // receive messages from connected peer and write them to stdout
                while (true)
                {
                    char buffer[BUFFSIZE + 1];
                    int n_recv = receive_msg(&accepted_socket, buffer);

                    if (n_recv > 0)
                        write_msg(&accepted_socket, buffer);
                    else if (n_recv == 0)
                        break;
                    else
                    {
                        // check if connection timed out
                        if (errno == EAGAIN || errno == EWOULDBLOCK)
                        {
                            printf("Client %i idle timeout\n", n_connections);
                            break;
                        }
                        error_handler(errno, "receive failed");
                    }
                }
                printf("Closing connection for client %i\n", n_connections);

                // child process cleanup
                close(accepted_socket.socket_fd);
                close(socket_fd);
                _exit(EXIT_SUCCESS);
            }
            else if (handler_pid > 0)
                n_connections++;
            else
                error_handler(errno, "fork failed");
        }
    }
    printf("No active connections, closing server\n");

    // cleanup
    close(socket_fd);
    return 0;
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
