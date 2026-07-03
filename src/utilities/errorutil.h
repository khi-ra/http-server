/* Print a message with `fprintf (stderr, FORMAT, ...)`.
 * If ERRNUM is nonzero, follow it with ": " and strerror (ERRNUM).
 * If STATUS is nonzero, terminate the program with `exit (STATUS)`.*/
void error_handler(int errnum, char* msg);
