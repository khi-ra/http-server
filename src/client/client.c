#include "../utilities/errorutil.h"
#include "../utilities/socketutil.h"
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

int read_input(char **line);

int main()
{
    struct sockaddr_in server_address;
    int setup_successful = 1;

    int socket_fd = create_tcp_ipv4_socket();
    server_address = create_ipv4_address("127.0.0.1", 8080);

    // send connection request to server
    if (connect(socket_fd, (struct sockaddr *) &server_address, sizeof(server_address)) == -1)
    {
        error_handler(errno, "connect failed");
        setup_successful = 0;
    }

    if (setup_successful)
        printf("Connection was successful\n");

    // read input from stdin and send to server
    char *line = NULL;
    printf("Enter input to send to the server ('exit' to stop):\n");

    while (setup_successful)
    {
        int n_read = read_input(&line);

        if (n_read == -1)
            error_handler(errno, "reading user input failed:");

        if (strcmp(line, "exit\n") == 0)
            break;

        if (send(socket_fd, line, n_read, 0) == -1)
            error_handler(errno, "sending message to server failed");
    }

    // cleanup
    close(socket_fd);
    free(line);
    return 0;
}

/* Read input line-by-line from stdin and store it in LINE.
 * Return number of bytes read or -1 for errors. */
int read_input(char **line)
{
    size_t line_size = 0;
    ssize_t n_read = getline(line, &line_size, stdin);

    if (n_read == -1 && (errno == EINVAL || errno == ENOMEM))
        return -1;

    return n_read;
}
