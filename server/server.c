#include "../utilities/socketutil.h"
#include <errno.h>
#include <error.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_CONNECTIONS 5

int recv_and_write_msg(struct accepted_socket *accepted_socket);

int main()
{
    int socket_fd;
    struct sockaddr_in *address = NULL;
    struct accepted_socket *accepted_socket;

    // create the server's socket fd and address
    create_ipv4_address(&address, "", 8080);
    if ((socket_fd = create_tcp_ipv4_socket()) == -1)
        error(EXIT_FAILURE, errno, "creating socket failed");

    // bind the server's socket fd and address
    if (bind(socket_fd, (struct sockaddr *) address, sizeof(struct sockaddr)) == -1)
        error(EXIT_FAILURE, errno, "bind failed");

    printf("Server socket was bound successfully\n");

    // listen for incoming connection requests, queue upto MAX_CONNECTIONS connections
    if ((listen(socket_fd, MAX_CONNECTIONS)) == -1)
        error(EXIT_FAILURE, errno, "listen failed");

    // accept connections and create a child process to handle each connection
    int n_children;
    for (int i = 0; i < MAX_CONNECTIONS; i++)
    {
        accepted_socket = accept_connection(socket_fd);

        if (accepted_socket->accepted)
        {
            if (!fork())
            {
                n_children++;

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
            }
        }
        else
            error(0, errno, "accept failed");
    }

    // parent waits for all children
    for (int i = 0; i < n_children; i++)
        wait(NULL);

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

        // converting ip and port to human readable format
        inet_ntop(accepted_socket->address.sin_family, &accepted_socket->address.sin_addr.s_addr, ip, sizeof(ip));
        port = ntohs(accepted_socket->address.sin_port);

        buffer[n_recv] = 0;
        printf("Message from %s:%hu = %s \n", ip, port, buffer);
    }

    return n_recv;
}
