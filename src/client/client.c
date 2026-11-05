#include "../utilities/errorutil.h"
#include "../utilities/socketutil.h"
#include <errno.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>

static int errnum;

static int read_input(char **line);

/* Read input from stdin and store it in LINE.
 *
 * Return number of bytes read or -1 on error. */
int read_input(char **line)
{
    size_t line_size = 0;
    int n_read = getline(line, &line_size, stdin);
    if (n_read < 0 && (errno == EINVAL || errno == ENOMEM))
        errnum = ERR_SOCK_IO;

    return n_read;
}

int main()
{
    char *line = NULL;

    struct sockaddr_in server_addr = create_ipv4_address("127.0.0.1", 8080);
    int socket_fd = create_tcp_ipv4_socket();
    if (socket_fd < 0)
        goto error_out;

    int result = connect(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr));
    if (result < 0)
        goto error_out;

    printf("Connection was successful\n");
    printf("Enter input to send to the server ('exit' to stop):\n");

    // read input from stdin and send to server
    while (true)
    {
        int n_read = read_input(&line);
        if (n_read < 0)
            break;
        if (strcmp(line, "exit\n") == 0)
            goto out;

        int n_sent = send(socket_fd, line, n_read, 0);
        if (n_sent < 0 && errnum == ERR_SOCK_IO)
            break;
    }

error_out:
    error_handler(errno, err_str(errnum));

out:
    close(socket_fd);
    if (line)
        free(line);
}
