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
#include "../../include/nmp.h"
#include "../../include/nmp_response.h"
#include "../../include/node_registry.h"
#include "../../include/state_manager.h"

#define UDP_BUF_SIZE 512

/* Serializa msg y lo envía como un datagrama al remitente original. */
static void send_udp_response(int sockfd, const struct sockaddr_in *sender,
                              const nmp_message_t *msg) {
    char out[UDP_BUF_SIZE];

    if (nmp_serialize_response(msg, out, sizeof(out)) != 0) {
        fprintf(stderr, "Error serializando respuesta NMP (UDP)\n");
        return;
    }

    if (sendto(sockfd, out, strlen(out), 0,
               (const struct sockaddr *)sender, sizeof(*sender)) < 0) {
        perror("sendto");
    }
}

void handle_udp_message(int sockfd, struct sockaddr_in *sender, const char *raw, size_t len) {
    char text[UDP_BUF_SIZE];
    nmp_message_t request;
    nmp_message_t response;

    /* El datagrama no trae '\0': se copia y se quitan '\n'/'\r' finales. */
    if (len >= sizeof(text)) {
        len = sizeof(text) - 1;
    }
    memcpy(text, raw, len);
    while (len > 0 && (text[len - 1] == '\n' || text[len - 1] == '\r')) {
        len--;
    }
    text[len] = '\0';

    if (nmp_parse(text, &request) != 0) {
        nmp_build_error(NULL, "INVALID_FORMAT", &response);
    } else if (request.type != NMP_STATUS) {
        /* Por UDP solo viaja STATUS (y su ACK de vuelta) — docx, sección 9. */
        nmp_build_error(&request, "INVALID_MESSAGE", &response);
    } else if (!is_node_registered(request.node_id)) {
        nmp_build_error(&request, "UNKNOWN_NODE", &response);
    } else {
        update_node_state(request.node_id, request.data);

        if (nmp_build_response(&request, &response) != 0) {
            nmp_build_error(&request, "INVALID_MESSAGE", &response);
        }
    }

    send_udp_response(sockfd, sender, &response);
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
