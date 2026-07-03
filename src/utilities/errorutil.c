#include <stdio.h>
#include <string.h>

void error_handler(int errnum, char* msg)
{
	if (msg)
		fprintf(stderr, "%s%s\n", msg, errnum == 0 ? "" : strcat(": ", strerror(errnum)));
	else if (errnum != 0)
		fprintf(stderr, "%s\n", strerror(errnum));
}
