#include "../socketutil.h"
#include <errno.h>

int main()
{
    // create server socket and address
    int server_socket_fd = create_tcp_ipv4_socket();
    if (server_socket_fd == -1)
    {
        fprintf(stderr, "Error encountered when creating TCP IPv4 socket, errno: %d\n", errno);
        return -1;
    }
    struct sockaddr_in *server_address = create_ipv4_address("", 49153);

    // bind server socket to server address
    int bind_flag = bind(server_socket_fd, (struct sockaddr *) server_address, sizeof(*server_address));
    if (bind_flag == -1)
    {
        fprintf(stderr, "Server socket bind was unsuccessful, errno: %d\n", errno);
        return -1;
    }
    else
        printf("Server socket was bound successfully\n");

    // listen for connection requests on server socket
    int listen_flag = listen(server_socket_fd, 5);
    if (listen_flag == -1)
    {
        fprintf(stderr, "Error encountered when listening for connections, errno: %d\n", errno);
        return -1;
    }

    // accept incoming request from client and open a socket to communicate
    struct sockaddr_in *client_address = NULL;
    socklen_t client_address_size = sizeof(struct sockaddr_in);

    int client_socket_fd = accept(server_socket_fd, (struct sockaddr *) client_address, &client_address_size);
    if (client_socket_fd == -1)
    {
        fprintf(stderr, "Error encountered when accepting connections, errno: %d\n", errno);
        return -1;
    }

    // receive message/request from the client
    char buffer[1024];
    int receive_flag = recv(client_socket_fd, buffer, 1024, 0);
    if (receive_flag == -1)
    {
        fprintf(stderr, "Error encountered when attemping to receive requests/messages, errno: %d\n", errno);
        return -1;
    }
    else
        printf("Response was: %s\n", buffer);

    free(server_address);
}
