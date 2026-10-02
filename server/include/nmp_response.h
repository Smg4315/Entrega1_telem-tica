#ifndef NMP_RESPONSE_H
#define NMP_RESPONSE_H

#include "nmp.h"
#include <stddef.h>

/*
 * Construye la respuesta NMP correspondiente a un mensaje recibido.
 *
 * Retorna:
 *   0  -> respuesta construida correctamente
 *  -1  -> mensaje inválido o parámetros incorrectos
 */
int nmp_build_response(
    const nmp_message_t *request,
    nmp_message_t *response
);

/* Nombre textual del tipo ("REGISTER", "ACK"...); NULL si no es válido. */
const char *nmp_type_to_string(nmp_msg_type_t type);

/*
 * Construye un ERROR con el código indicado (INVALID_FORMAT, INVALID_MESSAGE...).
 * request puede ser NULL si el mensaje no se pudo parsear: ID "0", NODE_ID vacío.
 */
void nmp_build_error(
    const nmp_message_t *request,
    const char *code,
    nmp_message_t *response
);

/*
 * Serializa un mensaje NMP para enviarlo mediante send_message()/sendto().
 * El ID se emite tal como llegó (id_raw); si está vacío se usa id numérico.
 *
 * ACK y ERROR:  TIPO|ID|DATOS|NODE_ID   ej. ACK|001|REGISTER|NODE01
 *                                           ERROR|101|UNKNOWN_NODE|NODE99
 * Resto:        TIPO|ID|NODE_ID|DATOS   ej. RESPONSE|101|NODE01|CURRENT
 */
int nmp_serialize_response(
    const nmp_message_t *message,
    char *out,
    size_t out_len
);

#endif