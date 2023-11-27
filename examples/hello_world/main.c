#include "../../src/surf.h"


void callback(SurfApp* server, int port) {
    printf("Listening on port %d...\n", port);
}

void handler(HttpRequest* request, HttpResponse* response) {
    send_resp(json(status(response, 200), "{\"hallo\": \"welt\"}"));
}

int main() {
    SurfApp* app = new_surf();
    get(app, "/", handler);
    surf_listen(app, 8081, callback);
    return 0;
}