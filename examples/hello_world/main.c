#include "../../src/surf.h"


void callback(SurfApp* server, int port) {
    printf("Listening on port %d...\n", port);
}

void home(HttpRequest* request, HttpResponse* response) {
    send_resp(html(status(response, 200), "<html>"
    "<h1>Welcome on Index</h1>"
    "<button>Click Me</button>"
    "</html>"));
}

void greetings(HttpRequest* request, HttpResponse* response) {
    send_resp(json(status(response, 200), "{\"greetings\": \"Hello World!\"}"));
}

int main() {
    SurfApp* app = new_surf();
    get(app, "/", home);
    get(app, "/greetings", greetings);
    surf_listen(app, 8084, callback);
    return 0;
}