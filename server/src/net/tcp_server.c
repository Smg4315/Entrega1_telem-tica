/* Servidor TCP: REGISTER, EVENT, QUERY, RESPONSE, ERROR (docx, sección 9 — Análisis de TCP y UDP).
 * socket-core: apertura de socket, bind, listen y loop de accept.
 * El parseo/serialización NMP (nmp_parse/nmp_serialize) no es responsabilidad
 * de este módulo — ver src/protocol/nmp_parser.c.
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define TCP_BACKLOG 10
#define TCP_BUF_SIZE 512

/* TODO: pertenece al módulo de parsing (src/protocol/nmp_parser.c). */
void handle_tcp_message(int client_fd, const char *raw, size_t len) {
    (void)client_fd;
    (void)raw;
    (void)len;
}

int start_tcp_server(int port) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return -1;
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(server_fd);
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons((uint16_t)port);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, TCP_BACKLOG) < 0) {
        perror("listen");
        close(server_fd);
        return -1;
    }

    return server_fd;
}

void run_tcp_accept_loop(int server_fd) {
    char buf[TCP_BUF_SIZE];

    for (;;) {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);

        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        ssize_t n = read(client_fd, buf, sizeof(buf));
        if (n > 0) {
            handle_tcp_message(client_fd, buf, (size_t)n);
        }

        close(client_fd);
    }
}
