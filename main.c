#include "socketutil.h"

int main()
{
    int socket_fd = create_tcp_ipv4_socket();

    struct sockaddr_in *address = NULL;
    create_ipv4_addr(address, "127.0.0.1", 2000);

    int result = connect(socket_fd, (struct sockaddr *) address, sizeof(*address));
    if (result == 0)
        printf("Connection was successful\n");

    // send GET request
    char *request;
    request = "GET / HTTP/1.1\r\nHost:google.com\r\n\r\n";
    send(socket_fd, request, strlen(request), 0);

    // receive response
    char response[1024];
    recv(socket_fd, response, 1024, 0);

    printf("response: %s\n", response);
}
