#include "httpparserutil.h"

request_t parse_request(const char* http_request_buffer)
{
	// create request object
	request_t request;
	request.header_count = 0;
	request.body_length = 0;

	// create parser object
	http_parser_t http_parser;
	http_parser.state = STATE_METHOD;
	http_parser.request = request;

	// loop over every character and send to parser_feed
	for (size_t index = 0; index < strlen(http_request_buffer); index++)
	{
		// check if character is CRLF and change to null terminator
		char c = http_request_buffer[index];
		if (http_request_buffer[index] == '\r' && http_request_buffer[index + 1] == '\n')
			c = '\0';
			
		if (parser_feed(&http_parser, c) == 0)
			break;
	}

	return http_parser.request;
}

// parse each character from request
int parser_feed(http_parser_t* http_parser, char c)
{
	switch(http_parser->state)
	{
		case STATE_METHOD:
			if (is_separator(c))
			{
				http_parser->index = 0;
				http_parser->state = STATE_URI;
				break;
			}

			http_parser->request.method[http_parser->index++] = c;
			break;
		case STATE_URI:
			if (is_separator(c))
			{
				http_parser->index = 0;
				http_parser->state = STATE_VERSION;
				break;
			}

			http_parser->request.uri[http_parser->index++] = c;
			break;
		case STATE_VERSION:
			if (c == '\0')
			{
				http_parser->index = 0;
				http_parser->state = STATE_HEADER_NAME;
				break;
			}

			http_parser->request.version[http_parser->index++] = c;
			break;
		case STATE_HEADER_NAME:
			if (c == '\0')
			{
				http_parser->index = 0;
				http_parser->state = STATE_BODY;
				break;
			}

			// check for header name end
			if (c == ':')
			{
				// create header object and write header name into it
				header_t header;
				for (size_t index = 0; index <= http_parser->index; index++)
				{
					header.name[index] = http_parser->buffer[index];
				}
				http_parser->request.headers[http_parser->request.header_count] = header;

				memset(http_parser->buffer, 0, sizeof(http_parser->buffer));
				http_parser->index = 0;
				http_parser->state = STATE_HEADER_VALUE;
				break;
			}

			http_parser->buffer[http_parser->index++] = c;
			break;
		case STATE_HEADER_VALUE:
			// dont parse anything if first character, since it is always a space
			if (http_parser->index == 0 && is_separator(c))
				break;

			if (c == '\0')
			{
				http_parser->request.header_count++;
				http_parser->index = 0;
				http_parser->state = STATE_HEADER_NAME;
				break;
			}

			http_parser->request.headers[http_parser->request.header_count].value[http_parser->index++] = c;
			break;
		case STATE_BODY:
			if (c == '\0')
			{
				http_parser->request.body_length = http_parser->index;
				http_parser->index = 0;
				http_parser->state = STATE_DONE;
				break;
			}

			http_parser->request.body[http_parser->index++] = c;
			break;
		case STATE_DONE:
			return 0;
		case STATE_ERROR:
			printf("error");
			return 0;
	}

	return 1;
}

// check for any separator characters
inline bool is_separator(char c)
{
	return c == ' ' || c == '\t' || c == '\f' || c == '\v' || c == '\r';
}
