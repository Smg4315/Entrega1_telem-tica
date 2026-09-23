#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int start_tcp_server(int port);
void run_tcp_accept_loop(int server_fd);
int start_udp_server(int port);
void run_udp_recv_loop(int sockfd);

/* El loop UDP bloquea igual que el de TCP: corre en su propio hilo. */
static void *udp_thread_main(void *arg) {
    run_udp_recv_loop(*(int *)arg);
    return NULL;
}

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

    int udp_fd = start_udp_server(port);

    if (udp_fd < 0) {
        fprintf(stderr, "Error: no se pudo iniciar el servidor UDP.\n");
        return 1;
    }

    pthread_t udp_thread;

    if (pthread_create(&udp_thread, NULL, udp_thread_main, &udp_fd) != 0) {
        fprintf(stderr, "Error: no se pudo crear el hilo UDP.\n");
        return 1;
    }

    printf("NetMonitor UDP escuchando en el puerto %d...\n", port);

    run_tcp_accept_loop(server_fd);

    return 0;
}