#include <stdio.h>
#include <stdlib.h>

int start_tcp_server(int port);
void run_tcp_accept_loop(int server_fd);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <puerto>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);

    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Error: puerto inválido.\n");
        return 1;
    }

    int server_fd = start_tcp_server(port);

    if (server_fd < 0) {
        fprintf(stderr, "Error: no se pudo iniciar el servidor.\n");
        return 1;
    }

    printf("NetMonitor TCP escuchando en el puerto %d...\n", port);

    run_tcp_accept_loop(server_fd);

    return 0;
}