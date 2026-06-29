#include "../utilities/socketutil.h"
#include <err.h>
#include <errno.h>
#include <unistd.h>

int read_input(char **line);

int main()
{
    int client_socket_fd = create_tcp_ipv4_socket();
    struct sockaddr_in *server_address = create_ipv4_address("127.0.0.1", 49153);

    // send connection request to server
    if (connect(client_socket_fd, (struct sockaddr *) server_address, sizeof(*server_address)) == -1)
        errx(EXIT_FAILURE, "Connecton unsuccessful");

    printf("Connection was successful\n");

    // read input from stdin and send to server
    char *line = NULL;
    printf("Enter input to send to the server ('exit' to stop):\n");

    while (true)
    {
        int n_read = read_input(&line);

        if (n_read == -1)
            errx(EXIT_FAILURE, "Error reading input from stdin");

        if (strcmp(line, "exit\n") == 0)
            break;

        if (send(client_socket_fd, line, n_read, 0) == -1)
            errx(EXIT_FAILURE, "Error sending message to server");
    }

    // close file and free memory
    close(client_socket_fd);
    free(server_address);
    free(line);
    return 0;
}

int read_input(char **line)
{
    size_t line_size = 0;
    ssize_t char_count = getline(line, &line_size, stdin);

    if (char_count == -1 && (errno == EINVAL || errno == ENOMEM))
        return -1;

    return char_count;
}
