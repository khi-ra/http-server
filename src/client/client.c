#include "../utilities/errorutil.h"
#include "../utilities/socketutil.h"
#include <asm-generic/errno.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>

static int errnum;

static int setup(int *fd, struct sockaddr_in *server_addr, char *ip, int port);
static int read_input(char **line);

int setup(int *fd, struct sockaddr_in *server_addr, char *ip, int port)
{
    int socket_fd;
    int err_flag = -1;

    *server_addr = create_ipv4_address(ip, port);
    socket_fd = create_tcp_ipv4_socket();
    if (socket_fd == -1)
        goto out;

    int connect_result = connect(socket_fd, (struct sockaddr *) &server_addr, sizeof(*server_addr));
    if (connect_result == -1)
        goto out;

    *fd = socket_fd;
    err_flag = 0;

out:
    if (err_flag == -1)
        errnum = ERR_SETUP;

    return err_flag;
}

int main()
{
    int socket_fd;
    struct sockaddr_in server_address;
    char *line = NULL;

    int setup_result = setup(&socket_fd, &server_address, "127.0.0.1", 8080);
    if (setup_result == -1)
        goto error_out;

    printf("Connection was successful\n");
    printf("Enter input to send to the server ('exit' to stop):\n");

    // read input from stdin and send to server
    while (true)
    {
        int n_read = read_input(&line);

        if (n_read == -1)
            break;

        if (strcmp(line, "exit\n") == 0)
            goto out;

        int n_sent = send(socket_fd, line, n_read, 0);
        if (n_sent == -1 && errnum == ERR_SOCK_IO)
            break;
    }

    error_handler(errno, err_str(errnum));

out:
    close(socket_fd);
    if (line)
        free(line);
}

/* Read input line-by-line from stdin and store it in LINE.
 * Return number of bytes read or -1 for errors. */
int read_input(char **line)
{
    size_t line_size = 0;
    int n_read = getline(line, &line_size, stdin);
    if (n_read == -1 && (errno == EINVAL || errno == ENOMEM))
        errnum = ERR_SOCK_IO;

    return n_read;
}
