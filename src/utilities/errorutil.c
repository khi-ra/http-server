#include "errorutil.h"

static const char *error_messages[] = {
    [ERR_SETUP] = "Server setup failed",
    [ERR_ACCEPT] = "Accepting connection failed",
    [ERR_SOCK_IO] = "Socket I/O failed",
    [ERR_SOCK_TIMEOUT] = "Socket idle timeout",
    [ERR_EVENTFD] = "Events file I/O failed",
    [ERR_POLL] = "Polling for connection failed",
    [ERR_THREAD] = "Creating/Detaching thread failed",
};

void error_handler(int errnum, const char *msg)
{
    if (msg)
        fprintf(stderr, "%s: %s\n", msg, errnum == 0 ? "" : strerror(errnum));
    else if (errnum != 0)
        fprintf(stderr, "%s\n", strerror(errnum));
}

const char *err_str(int errnum)
{
    return error_messages[errnum];
}
