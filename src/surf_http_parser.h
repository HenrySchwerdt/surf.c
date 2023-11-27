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
char* extract_path(const char* request, size_t request_length) {
    char* path = (char*) malloc(sizeof(char));
    int count = 0;
    int found = 0;
    for (int i = 0; i < request_length; i++) {
        if (request[i] == ' ' && found) {
            break; 
        }
        if (request[i] == ' ' && !found) {
            found = 1;
        }
        else if (found) {
            path[count] = request[i];
            path = realloc(path, count+1);
            count +=1;
        }
    }
    path = realloc(path, count+1);
    path[count] = '\0';
    return path;
}
HttpRequest *parse_http_request(int client_socket)
{
    
    Buffer* tcp_buffer = init_buffer();
    int length = read_line(client_socket, tcp_buffer);

    char* path = extract_path(tcp_buffer->buffer, length);
    HttpRequest* req = (HttpRequest*)malloc(sizeof(HttpRequest));
    req->path = path;
    free_buffer(tcp_buffer);
    return req;
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