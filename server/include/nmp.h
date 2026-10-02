#ifndef NMP_H
#define NMP_H

#include <stddef.h>

/* Tipos de mensaje NMP (docx, sección 4) */
typedef enum {
    NMP_REGISTER,
    NMP_STATUS,
    NMP_EVENT,
    NMP_ACK,
    NMP_QUERY,
    NMP_RESPONSE,
    NMP_ERROR
} nmp_msg_type_t;

/* Mensaje NMP: TIPO|ID|NODE_ID|DATOS */
typedef struct {
    nmp_msg_type_t type;
    unsigned int    id;
    char            id_raw[16];   /* ID tal como llegó (conserva ceros: "001") */
    char            node_id[32];
    char            data[256];
} nmp_message_t;

/* Estructura válida (campos e ID) pero TIPO desconocido -> INVALID_MESSAGE. */
#define NMP_PARSE_UNKNOWN_TYPE (-2)

/* Parser del protocolo NMP. Debe aceptar mensajes con payload que puede incluir
 * separadores | a partir del campo DATOS.
 *   0  -> mensaje válido.
 *  -1  -> no se pudo parsear (campos faltantes, ID no numérico...): INVALID_FORMAT.
 *  NMP_PARSE_UNKNOWN_TYPE -> out trae id/node_id/data, pero out->type no es válido. */
int nmp_parse(const char *raw, nmp_message_t *out);

#endif
