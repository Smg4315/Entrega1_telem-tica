#ifndef MSG_IO_H
#define MSG_IO_H

#include <stddef.h>

/* Transporte de mensajes NMP sobre TCP (docx, sección 4.2): framing por
 * delimitador '\n' — cada mensaje de texto TIPO|ID|NODE_ID|DATOS termina en
 * un salto de línea.
 *
 * recv_message: bloquea hasta completar un mensaje, ver FIN o error.
 *   Retorno > 0 -> bytes del mensaje en buf (sin '\n', con '\0' final).
 *   Retorno == 0 -> el peer cerró la conexión (FIN), sin mensaje completo.
 *   Retorno == -1 -> error real de socket (revisar errno reportado por perror).
 *
 * send_message: envía msg completo (agrega '\n' si no lo trae) reintentando
 * hasta escribir todos los bytes.
 *   Retorno 0 -> enviado completo.
 *   Retorno -1 -> error real de socket.
 */
ssize_t recv_message(int fd, char *buf, size_t maxlen);
int send_message(int fd, const char *msg);

#endif
