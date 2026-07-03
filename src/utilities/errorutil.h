#ifndef ERRORUTIL_H
#define ERRORUTIL_H

#include <stdio.h>
#include <string.h>

/* Print a message with `fprintf (stderr, FORMAT, ...)`.
 * If ERRNUM is nonzero, follow it with ": " and `strerror(ERRNUM)`.
 * If STATUS is nonzero, terminate the program with `exit(STATUS)`.*/
void error_handler(int errnum, char *msg);

#endif
