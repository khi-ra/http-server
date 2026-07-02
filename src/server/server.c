#include "../utilities/socketutil.h"
#include <asm-generic/errno-base.h>
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAXCONN 5

int recv_and_write_msg(struct accepted_socket *accepted_socket);
void sigchld_handler(int sig_num);
void init_sigchld_action(struct sigaction *sigchld_action);

int main()
{
    int socket_fd;
    struct sockaddr_in *address = NULL;
    struct accepted_socket *accepted_socket;
    struct sigaction sigchld_action;

    // create server's TCP socket
    create_ipv4_address(&address, "", 8080);
    if ((socket_fd = create_tcp_ipv4_socket()) == -1)
        error(EXIT_FAILURE, errno, "creating socket failed");

    if (bind(socket_fd, (struct sockaddr *) address, sizeof(struct sockaddr)) == -1)
        error(EXIT_FAILURE, errno, "bind failed");

    printf("Server socket successfully created\n");

    if ((listen(socket_fd, MAXCONN)) == -1)
        error(EXIT_FAILURE, errno, "listen failed");

    // initialise a custom sigaction struct and set it as SIGCHLD's action
    init_sigchld_action(&sigchld_action);
    if (sigaction(SIGCHLD, &sigchld_action, NULL) == -1)
        error(EXIT_FAILURE, errno, "sigaction failed");

    // accept connections and handle connected client's requests
    int n_children = 0;

    while (true)
    {
        printf("n_children: %d\n ", n_children);

        // set the polling duration based on the number of active connections
        int timeout_ms = 0;
        if (n_children == 0)
            timeout_ms = SOCKET_TIMEOUT_MS;
        else if (n_children > 0)
            timeout_ms = -1;
        else
            break;

        accepted_socket = accept_connection(socket_fd, timeout_ms);

        // if poll() or accept() have been interrupted by SIGCHLD
        if (!accepted_socket->accepted && errno == EINTR)
            n_children--;
        else if (accepted_socket->accepted)
        {
            printf("Connection successfully received\n");

            pid_t handler_pid = fork();
            if (handler_pid == 0)
            {
                // receive messages from connected peer and write them to stdout
                while (true)
                {
                    int n_recv = recv_and_write_msg(accepted_socket);

                    if (n_recv == -1)
                        error(0, errno, "receive failed");
                    else if (n_recv == 0)
                        break;
                }

                close(accepted_socket->socket_fd);
                free(accepted_socket);
                _exit(EXIT_SUCCESS);
            }
            else if (handler_pid > 0)
                n_children++;
            else
                error(0, errno, "fork failed");
        }
    }

    printf("No active connections, closing server...\n");

    // close files and free memory
    close(socket_fd);
    free(address);
    if (accepted_socket)
        free(accepted_socket);

    return 0;
}

/* Receive message from SOCK_FD and print it to stdout. Return number
 * of bytes received or -1 for error. */
int recv_and_write_msg(struct accepted_socket *accepted_socket)
{
    char buffer[1024];
    int n_recv = recv(accepted_socket->socket_fd, buffer, sizeof(buffer), 0);

    if (n_recv > 0)
    {
        char ip[INET_ADDRSTRLEN];
        short port;

        // convert ip and port to human readable format
        inet_ntop(accepted_socket->address.sin_family, &accepted_socket->address.sin_addr.s_addr, ip, sizeof(ip));
        port = ntohs(accepted_socket->address.sin_port);

        buffer[n_recv] = 0;
        printf("Message from %s:%hu = %s \n", ip, port, buffer);
    }

    return n_recv;
}

/* Custom handler for signal SIGCHLD. */
void sigchld_handler(int sig_num)
{
    // quiet unused variable warning
    (void) sig_num;

    int saved_errno = errno;

    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;

    errno = saved_errno;
}

/* Initialise SIGCHLD_ACTION. */
void init_sigchld_action(struct sigaction *sigchld_action)
{
    sigchld_action->sa_handler = sigchld_handler;
    sigchld_action->sa_flags = 0;
    sigemptyset(&sigchld_action->sa_mask);
}
