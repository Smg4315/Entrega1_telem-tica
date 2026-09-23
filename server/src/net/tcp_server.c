/* Servidor TCP: REGISTER, EVENT, QUERY, RESPONSE, ERROR (docx, sección 9 — Análisis de TCP y UDP).
 * socket-core: apertura de socket, bind, listen y loop de accept.
 * El parseo/serialización NMP (nmp_parse/nmp_serialize) no es responsabilidad
 * de este módulo — ver src/protocol/nmp_parser.c.
 */
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../../include/msg_io.h"
#include "../../include/nmp.h"
#include "../../include/nmp_response.h"
#include "../../include/node_registry.h"
#include "../../include/state_manager.h"
#include "../../include/logger.h"

#define TCP_BACKLOG 10
#define TCP_BUF_SIZE 512

void handle_tcp_message(int client_fd, const char *raw, size_t len) {
    nmp_message_t request;
    nmp_message_t response;
    char response_buffer[TCP_BUF_SIZE];

    (void)len;

    int parsed = (nmp_parse(raw, &request) == 0);

    if (!parsed) {
        nmp_build_error(NULL, "INVALID_FORMAT", &response);
    } else if (request.type != NMP_REGISTER && !is_node_registered(request.node_id)) {
        nmp_build_error(&request, "UNKNOWN_NODE", &response);
    } else {
        if (request.type == NMP_REGISTER) {
            register_node(request.node_id);
        } else if (request.type == NMP_STATUS || request.type == NMP_EVENT) {
            update_node_state(request.node_id, request.data);
        }

        if (nmp_build_response(&request, &response) != 0) {
            nmp_build_error(&request, "INVALID_MESSAGE", &response);
        }
    }

    if (nmp_serialize_response(
            &response,
            response_buffer,
            sizeof(response_buffer)) != 0) {

        fprintf(stderr, "Error serializando respuesta NMP\n");
        snprintf(response_buffer, sizeof(response_buffer), "SERIALIZE_ERROR");
    } else if (send_message(client_fd, response_buffer) < 0) {
        perror("send_message");
    }

    log_request(parsed ? request.node_id : NULL,
                parsed ? nmp_type_to_string(request.type) : "INVALID",
                response_buffer);
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

/* Recolecta hijos terminados para que no queden zombis. */
static void reap_children(int sig) {
    int saved_errno = errno;
    (void)sig;
    while (waitpid(-1, NULL, WNOHANG) > 0) {
    }
    errno = saved_errno;
}

static void serve_client(int client_fd) {
    char buf[TCP_BUF_SIZE];

    /* Una conexión puede traer varios mensajes seguidos hasta que el
     * cliente haga FIN (n==0) o ocurra un error real (n==-1). */
    for (;;) {
        ssize_t n = recv_message(client_fd, buf, sizeof(buf));
        if (n > 0) {
            handle_tcp_message(client_fd, buf, (size_t)n);
            continue;
        }
        if (n == 0) {
            break; /* el cliente cerró su lado de escritura */
        }
        perror("recv_message");
        break; /* error real: cerrar esta conexión sin tumbar el servidor */
    }
}

void run_tcp_accept_loop(int server_fd) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = reap_children;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa, NULL) < 0) {
        perror("sigaction");
    }

    for (;;) {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);

        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (client_fd < 0) {
            if (errno != EINTR) {
                perror("accept");
            }
            continue;
        }

        /* Un proceso hijo por conexión: un cliente lento no bloquea a los demás. */
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            close(client_fd);
            continue;
        }

        if (pid == 0) {
            close(server_fd);
            serve_client(client_fd);
            close(client_fd);
            _exit(0);
        }

        close(client_fd); /* el padre no usa la conexión del hijo */
    }
}
