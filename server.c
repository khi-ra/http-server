#include "socketutil.h"

int main()
{
    int server_socket_fd = create_tcp_ipv4_socket();
    struct sockaddr_in *server_address = NULL;
    create_ipv4_addr(server_address, "", 2000);

    if (bind(server_socket_fd, (struct sockaddr *) server_address, sizeof(*server_address) == 0))
        printf("server socket was bound successfully\n");

    // listen for connection requests on server_socket_fd, queueing upto 5 requests
    listen(server_socket_fd, 5);

    struct sockaddr_in *client_address;
    socklen_t client_address_size = sizeof(struct sockaddr_in);
    int client_socket_fd = accept(server_socket_fd, (struct sockaddr *) client_address, &client_address_size);

    // receive message/request from the client
    char buffer[1024];
    recv(client_socket_fd, buffer, 1024, 0);

    printf("response was: %s\n", buffer);
}
