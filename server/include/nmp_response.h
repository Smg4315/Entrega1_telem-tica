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

/*
 * Serializa un mensaje NMP para enviarlo mediante send_message().
 *
 * Ejemplo:
 * ACK|001|NODE01|REGISTER
 */
int nmp_serialize_response(
    const nmp_message_t *message,
    char *out,
    size_t out_len
);

#endif