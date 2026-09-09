#ifndef NMP_H
#define NMP_H

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
    unsigned int   id;
    char           node_id[32];
    char           data[256];
} nmp_message_t;

/* TODO: implementar en src/protocol/nmp_parser.c
   int nmp_parse(const char *raw, nmp_message_t *out);
   int nmp_serialize(const nmp_message_t *msg, char *out, size_t out_len);
*/

#endif
