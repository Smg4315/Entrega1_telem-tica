#ifndef LOGGER_H
#define LOGGER_H

/* Agrega una línea con marca de tiempo a logs/server.log (relativo al
 * directorio de trabajo; crea logs/ si no existe). Seguro entre hilos y
 * procesos: cada línea se escribe con un único write() en modo O_APPEND. */
void log_request(const char *node_id, const char *msg_type, const char *result);

#endif
