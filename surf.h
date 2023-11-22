#ifndef surf_h
#define surf_h

#include <stddef.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>


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
    char* path;
    char* body;
} HttpRequest;

typedef struct {
    int status_code;
    char* body;
} HttpResponse;

typedef HttpResponse (*RequestHandler)(HttpRequest);



typedef struct {
    char* method;
    char* path;
    RequestHandler handler;
} SurfRoute;

typedef struct {
    size_t num_routes;
    SurfRoute* routes;
} SurfApp;

typedef void (*StartUpCallback)(SurfApp, int);




#define SURF() (SurfApp) { .num_routes = 0, .routes = NULL }

#define ADD_ROUTE(server, method, path, handler) \
    do { \
        (server).routes = realloc((server).routes, ((server).num_routes + 1) * sizeof(Route)); \
        (server).routes[(server).num_routes].method = method; \
        (server).routes[(server).num_routes].path = path; \
        (server).routes[(server).num_routes].handler = handler; \
        (server).num_routes++; \
    } while (0)

#define USE(server, path, handler) ADD_ROUTE(server, "*", path, handler)
#define GET(server, path, handler) ADD_ROUTE(server, "get", path, handler)
#define PATCH(server, path, handler) ADD_ROUTE(server, "patch", path, handler)
#define PUT(server, path, handler) ADD_ROUTE(server, "put", path, handler)
#define POST(server, path, handler) ADD_ROUTE(server, "post", path, handler)
#define DELETE(server, path, handler) ADD_ROUTE(server, "post", path, handler)

#define _HANDLE_REQUEST(server, req) \
    ({ \
        HttpResponse response = { .status_code = 404, .body = "Not Found" }; \
        for (size_t i = 0; i < (server).num_routes; i++) { \
            if (strcmp((server).routes[i].method, (req).method) == 0 \
                && strcmp((server).routes[i].path, (req).path) == 0) { \
                response = (server).routes[i].handler(req); \
                break; \
            } \
        } \
        response; \
    })

#define LISTEN(server, port, callback) \
    do { \
        callback(server, port); \
        int server_socket = socket(AF_INET, SOCK_STREAM, 0); \
        if (server_socket == -1) { \
            perror("Error creating socket"); \
            exit(EXIT_FAILURE); \
        } \
        struct sockaddr_in server_address; \
        server_address.sin_family = AF_INET; \
        server_address.sin_port = htons(port); \
        server_address.sin_addr.s_addr = INADDR_ANY; \
        if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) { \
            perror("Error binding socket"); \
            exit(EXIT_FAILURE); \
        } \
        if (listen(server_socket, SOMAXCONN) == -1) { \
            perror("Error listening on socket"); \
            exit(EXIT_FAILURE); \
        } \
        while (1) { \
            int client_socket = accept(server_socket, NULL, NULL); \
            if (client_socket == -1) { \
                perror("Error accepting connection"); \
                continue; \
            } \
            char buffer[1024]; \
            ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer), 0); \
            if (bytes_received == -1) { \
                perror("Error receiving data"); \
            } else if (bytes_received == 0) { \
                printf("Client disconnected\n"); \
            } else { \
                printf("Received data from client: %.*s\n", (int)bytes_received, buffer); \
            } \
            close(client_socket); \
        } \
        close(server_socket); \
    } while (0)
   





#endif