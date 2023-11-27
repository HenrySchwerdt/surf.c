#ifndef surf_types_h
#define surf_types_h
#include <stdlib.h>
#include <stddef.h>
typedef struct {
    char *name;
    char *value;
} Header;

typedef enum {
    GET, PATCH, POST, PUT, DELETE
} Method;

typedef struct {
    Method method;
    Header* headers;
    int num_headers;
    char* path;
    char* body;
} HttpRequest;

typedef struct {
    int status_code;
    int client_fd;
    char* body;
    char* type;
} HttpResponse;

HttpResponse* status(HttpResponse* resp, int status) {
    resp->status_code = status;
    resp->body = NULL;
    return resp;
}

HttpResponse* html(HttpResponse* resp, const char *body) {
    resp->type = "text/html";
    resp->body = body;
    return resp;
}

HttpResponse* json(HttpResponse* resp, const char* body) {
    resp->type = "application/json";
    resp->body = body;
    return resp;
}

HttpResponse* text(HttpResponse* resp , const char *body){
    resp->type = "text/plain";
    resp->body = body;
    return resp;
}






#endif