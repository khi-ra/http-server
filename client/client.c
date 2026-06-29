#include "../socketutil.h"
#include <asm-generic/errno-base.h>
#include <errno.h>
#include <stdbool.h>

int main()
{
    int client_socket_fd = create_tcp_ipv4_socket();
    struct sockaddr_in *server_address = create_ipv4_address("127.0.0.1", 49153);

    // send connection request to server
    int connect_flag = connect(client_socket_fd, (struct sockaddr *) server_address, sizeof(*server_address));
    if (connect_flag == -1)
    {
        fprintf(stderr, "Connecton unsuccessful, errno: %d\n", errno);
        return -1;
    }
    else
        printf("Connection was successful\n");

    // read input from stdin and send to server
    char *line = NULL;
    size_t line_size = 0;
    printf("Enter input to send to the server ('exit' to stop):\n");
    /* NOTE: getline() returns -1 on EOF and on error. To differentiate, I
     * could also check value of errno whenever -1 is returned, as there are
     * only 2 possible errno values for getline(): EINVAL and ENOMEM.
     */
    while (true)
    {
        ssize_t char_count = getline(&line, &line_size, stdin);
        if (char_count == -1)
        {
            if (errno == EINVAL || errno == ENOMEM)
            {
                fprintf(stderr, "Error encountered when reading input from stdin, errno: %d", errno);
                return -1;
            }

            // if char_count == -1 is signalling EOF
            break;
        }
        else if (strcmp(line, "exit") == 0)
            break;

        send(client_socket_fd, line, char_count, 0);
    }

    // receive response
    char response[1024];
    int recv_flag = recv(client_socket_fd, response, 1024, 0);
    if (recv_flag == -1)
    {
        fprintf(stderr, "Error encountered when receiving message/response from server, errno: %d\n", errno);
        return -1;
    }
    else
        printf("Server response: %s\n", response);

    free(server_address);
    free(line);
}
