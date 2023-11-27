#ifndef http_parser_h
#define http_parser_h
#include "surf_types.h"
#include "surf_tcp_buffer.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>

typedef enum
{
    PARSE_METHOD,
    PARSE_PATH,
    PARSE_HEADERS,
    PARSE_BODY
} ParseState;

HttpRequest *parse_http_request(int client_socket)
{
    
    Buffer* tcp_buffer = init_buffer();
    int length;
    while((length = read_line(client_socket, tcp_buffer)) != -1) {
        printf("Received: %.*s\n", length, tcp_buffer->buffer);
    }
    free_buffer(tcp_buffer);
    return NULL;
}
// Function to free resources associated with HttpRequest
void free_http_request(HttpRequest *request)
{
    // Free headers
    for (size_t i = 0; i < request->num_headers; ++i)
    {
        free(request->headers[i].name);
        free(request->headers[i].value);
    }
    free(request->headers);

    // Free path and body
    free(request->path);
    free(request->body);

    // Free HttpRequest structure
    free(request);
}

#endif