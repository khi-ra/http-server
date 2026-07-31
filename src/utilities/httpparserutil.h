#include <stdio.h>
#include <string.h>

static const int MAX_HEADERS = 10;
static const int MAX_BODY = 2048;

static const char* CRLF = "\r\n";

// current parser state enum
typedef enum
{
	// request line
	STATE_METHOD,
	STATE_URI,
	STATE_VERSION,
	
	// headers
	STATE_HEADER_NAME,
	STATE_HEADER_VALUE,

	// message body
	STATE_BODY,
	
	// overall state
	STATE_DONE,
	STATE_ERROR
} parser_state_t;

// header struct
typedef struct
{
	char name[64];
	char value[256];
} header_t;

// request struct
typedef struct
{
	// request line
	char method[16];
	char uri[256];
	char version[16];

	// headers
	header_t headers[MAX_HEADERS];
	size_t header_count;

	// message body
	char body[MAX_BODY];
	size_t body_length;
} request_t;

// http parser struct
typedef struct
{
	parser_state_t state;
	size_t index;
	char buffer[1024];
	request_t request;
} http_parser_t;

// initial parser start
request_t parse_request(const char* http_request);
// start reading
int parser_feed(http_parser_t* http_parser, char c);
// check separator
inline bool is_separator(char c);
