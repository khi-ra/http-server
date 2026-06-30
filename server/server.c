#include "../utilities/socketutil.h"
#include <err.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

struct accepted_socket *accept_connection(int server_socket_fd);

int main()
{
    int server_socket_fd;
    struct sockaddr_in *server_address;
    struct accepted_socket *client_socket;

    // create server socket
    server_address = create_ipv4_address("", 49153);
    server_socket_fd = create_tcp_ipv4_socket();
    if (server_socket_fd == -1)
        errx(EXIT_FAILURE, "Error encountered when creating TCP IPv4 socket");

    // bind server socket to server address
    int bind_flag = bind(server_socket_fd, (struct sockaddr *) server_address, sizeof(*server_address));
    if (bind_flag == -1)
        errx(EXIT_FAILURE, "Server socket could not be bound");

    printf("Server socket was bound successfully\n");

    // accept any incoming requests
    if ((listen(server_socket_fd, 5)) == -1)
        errx(EXIT_FAILURE, "Error encountered when listening for connections");

    client_socket = accept_connection(server_socket_fd);
    if (!client_socket->accepted)
        errx(EXIT_FAILURE, "Error accepting incoming connection");

    // create child process to handle accepted client
    if (!fork())
    {
        // receive message from the client and print to stdout
        char buffer[1024];
        while (true)
        {
            int n_recv = recv(client_socket->socket_fd, buffer, sizeof(buffer), 0);

            if (n_recv > 0)
            {
                buffer[n_recv] = 0;
                printf("Response was: %s\n", buffer);
            }

            if (n_recv == -1)
                errx(EXIT_FAILURE, "Error receiving requests/messages");

            if (n_recv == 0)
                break;
        }
    }
    else
        wait(NULL);

    // close files and free memory
    close(client_socket->socket_fd);
    close(server_socket_fd);
    free(server_address);
    free(client_socket);
    return 0;
}
