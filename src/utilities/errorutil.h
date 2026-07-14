#ifndef ERRORUTIL_H
#define ERRORUTIL_H

#include <stdio.h>
#include <string.h>

enum error_codes
{
    ERR_SETUP,        /* Creating socket fd, binding, or listening failed. */
    ERR_ACCEPT,       /* Accepting the incoming connection failed. */
    ERR_SOCK_IO,      /* Sending or receiving message over the connection failed. */
    ERR_SOCK_TIMEOUT, /* Socket timed out, client was inactive for 60s. */
    ERR_EVENTFD,      /* Writing to or reading from the events file failed.  */
    ERR_POLL,         /* Polling failed. */
    ERR_THREAD,       /* Creating or detaching thread failed. */
};

/* Print MSG and ERRNUM's associated error message to stderr. If MSG is a null
 * pointer, print only ERRNUM's error message. If ERRNUM is zero, print MSG
 * followed by a colon and a space. */
void error_handler(int errnum, const char *msg);

/* Return the error message associated with ERRNUM. Can only be used for custom
 * error codes listed in the enum `error_codes`.
 */
const char *err_str(int errnum);

#endif
