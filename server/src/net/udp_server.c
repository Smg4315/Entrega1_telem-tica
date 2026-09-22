/* Servidor UDP: STATUS (periódico, tolera pérdida ocasional — docx, sección 9).
 * socket-core: apertura de socket, bind y loop de recvfrom.
 * El parseo/serialización NMP (nmp_parse/nmp_serialize) no es responsabilidad
 * de este módulo — ver src/protocol/nmp_parser.c.
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define UDP_BUF_SIZE 512

/* TODO: pertenece al módulo de parsing (src/protocol/nmp_parser.c). */
void handle_udp_message(int sockfd, struct sockaddr_in *sender, const char *raw, size_t len) {
    (void)sockfd;
    (void)sender;
    (void)raw;
    (void)len;
}

int start_udp_server(int port) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons((uint16_t)port);

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

void run_udp_recv_loop(int sockfd) {
    char buf[UDP_BUF_SIZE];

    for (;;) {
        struct sockaddr_in sender_addr;
        socklen_t addr_len = sizeof(sender_addr);

        ssize_t n = recvfrom(sockfd, buf, sizeof(buf), 0,
                              (struct sockaddr *)&sender_addr, &addr_len);
        if (n > 0) {
            handle_udp_message(sockfd, &sender_addr, buf, (size_t)n);
        }
    }
}
