#ifndef surf_h
#define surf_h
#include "surf_types.h"
#include "surf_http_parser.h"
#include <stddef.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

typedef void (*RequestHandler)(HttpRequest*, HttpResponse*);

typedef struct
{
    char *method;
    char *path;
    RequestHandler handler;
} SurfRoute;

typedef struct
{
    size_t num_routes;
    SurfRoute *routes;
} SurfApp;

typedef void (*StartUpCallback)(SurfApp*, int);

typedef struct
{
    SurfRoute *routes;
    int client_fd;
    int num_routes;
} ClientInfo;

SurfApp* new_surf() {
    SurfApp* app = (SurfApp*) malloc(sizeof(SurfApp));
    app->num_routes = 0;
    app->routes = NULL;
    return app;
}

void add_route(SurfApp* app, const char* method, const char* path, RequestHandler handler) {
    app->num_routes++;
    app->routes = realloc(app->routes, app->num_routes * sizeof(SurfRoute));

    app->routes[app->num_routes-1].method = strdup(method);
    app->routes[app->num_routes-1].path = strdup(path);

    if (app->routes[app->num_routes-1].method == NULL || app->routes[app->num_routes-1].path == NULL) {
        perror("Error allocating memory for method or path");
        return;
    }

    app->routes[app->num_routes-1].handler = handler;
}

void use(SurfApp* app, const char * path, RequestHandler handler) {
    add_route(app, "USE", path, handler);
}

void get(SurfApp* app, const char * path, RequestHandler handler) {
    add_route(app, "GET", path, handler);
}

void patch(SurfApp* app, const char * path, RequestHandler handler) {
    add_route(app, "PATCH", path, handler);
}

void put(SurfApp* app, const char * path, RequestHandler handler) {
    add_route(app, "PUT", path, handler);
}

void post(SurfApp* app, const char * path, RequestHandler handler) {
    add_route(app, "POST", path, handler);
}

void delete(SurfApp* app, const char * path, RequestHandler handler) {
    add_route(app, "DELETE", path, handler);
}



void send_resp(HttpResponse *resp)
{
    const char *response_format = "HTTP/1.1 %d\r\n"
                                  "Content-Type: %s\r\n"
                                  "Content-Length: %zu\r\n"
                                  "\r\n"
                                  "%s";
    size_t response_size = snprintf(NULL, 0, response_format, resp->status_code, "json", strlen(resp->body), resp->body) + 1;
    char *response = malloc(response_size);
    if (response == NULL)
    {
        perror("Failed to allocate memory for response");
        exit(EXIT_FAILURE);
    }
    snprintf(response, response_size, response_format, resp->status_code, "json", strlen(resp->body), resp->body);

    // Send the HTTP response to the client
    ssize_t bytes_sent = send(resp->client_fd, response, response_size - 1, 0);
    if (bytes_sent == -1)
    {
        perror("Error sending response");
    }
    free(response);
    free(resp);
}

void *handle_client(void *arg)
{
    ClientInfo *client_info = (ClientInfo *)arg;
    // HttpRequest* request = parse_http_request(client_fd);
    HttpResponse* resp = (HttpResponse*) malloc(sizeof(HttpResponse));
    resp->client_fd = client_info->client_fd;
    if (!client_info->num_routes)
    {
        send_resp(html(status(resp, 404),  "<!DOCTYPE html>\n"
                       "<html>\n"
                       "<head>\n"
                       "    <title>404 Not Found</title>\n"
                       "</head>\n"
                       "<body>\n"
                       "    <h1>404 Not Found</h1>\n"
                       "    <p>The requested resource could not be found.</p>\n"
                       "</body>\n"
                       "</html>"));
    } else {
        client_info->routes[0].handler(NULL, resp);
    }
    
    free(client_info);
}

void surf_listen(SurfApp* app, int port, StartUpCallback callback)
{
    callback(app, port);
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }
    if (listen(server_socket, SOMAXCONN) == -1)
    {
        perror("Error listening on socket");
        exit(EXIT_FAILURE);
    }
    while (1)
    {
        struct sockaddr_in client_adr;
        socklen_t client_adr_len = sizeof(client_adr);
        int *client_fd = malloc(sizeof(int));

        if ((*client_fd = accept(server_socket, (struct sockaddr *)&client_adr, &client_adr_len)) < 0)
        {
            perror("Accept failed");
            continue;
        }
        ClientInfo *client_info = malloc(sizeof(ClientInfo));
        if (client_info == NULL) {
            perror("Error allocating memory for client info");
            close(*client_fd);  
            continue;
        }
        client_info->routes = app->routes;
        client_info->num_routes = app->num_routes;
        client_info->client_fd = *client_fd;
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_client, (void *)client_info);
        pthread_detach(thread_id);
    }
    close(server_socket);
}

#endif