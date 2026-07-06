#ifndef ERRORUTIL_H
#define ERRORUTIL_H

#include <stdio.h>
#include <string.h>

/* Print MSG and ERRNUM's associated error message to stderr.
 * If MSG is a null pointer, print only ERRNUM's error message.
 * If ERRNUM is zero, print MSG followed by a colon and a space. */
void error_handler(int errnum, char *msg);

#endif
