#include "../../surf.h"


void callback(SurfApp server, int port) {
    printf("Listening on port %d...\n", port);
}

int main() {
    SurfApp app = SURF();
    LISTEN(app, 9005, callback);
    return 0;
}