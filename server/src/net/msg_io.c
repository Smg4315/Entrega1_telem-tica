/* Transporte de mensajes (I/O) sobre socket TCP ya conectado.
 * Implementa recv_message()/send_message() descritos en include/msg_io.h.
 */
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include "../../include/msg_io.h"

ssize_t recv_message(int fd, char *buf, size_t maxlen) {
    if (maxlen == 0) {
        return -1;
    }

    size_t used = 0;

    /* Lee byte a byte hasta el delimitador '\n' para no consumir bytes
     * del siguiente mensaje que puedan venir pegados en el mismo flujo. */
    while (used < maxlen - 1) {
        char c;
        ssize_t n = recv(fd, &c, 1, 0);

        if (n > 0) {
            if (c == '\n') {
                break;
            }
            buf[used++] = c;
            continue;
        }

        if (n == 0) {
            /* FIN: el peer cerró su lado de escritura.
             * Si no había nada acumulado, es un cierre "limpio" entre mensajes.
             * Si había bytes a medias, el mensaje quedó incompleto igual. */
            buf[used] = '\0';
            return 0;
        }

        /* n == -1: revisar errno */
        if (errno == EINTR) {
            continue; /* syscall interrumpida por una señal, no es un error real */
        }

        return -1; /* error real (ej. ECONNRESET) */
    }

    buf[used] = '\0';
    return (ssize_t)used;
}

int send_message(int fd, const char *msg) {
    size_t len = strlen(msg);
    int need_newline = (len == 0 || msg[len - 1] != '\n');

    size_t total_len = len + (need_newline ? 1 : 0);
    size_t sent = 0;

    while (sent < total_len) {
        const char *chunk;
        size_t chunk_len;

        if (sent < len) {
            chunk = msg + sent;
            chunk_len = len - sent;
        } else {
            chunk = "\n";
            chunk_len = 1;
        }

        ssize_t n = send(fd, chunk, chunk_len, 0);

        if (n > 0) {
            sent += (size_t)n; /* escritura parcial: se reintenta con el resto */
            continue;
        }

        if (n == -1 && errno == EINTR) {
            continue;
        }

        return -1; /* error real (ej. EPIPE si el peer ya cerró) */
    }

    return 0;
}
