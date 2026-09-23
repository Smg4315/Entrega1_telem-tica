/* Parsing del protocolo NMP: TIPO|ID|NODE_ID|DATOS (docx, sección 4).
 * Debe ser testeable sin abrir sockets reales (ver tests/).
 */
#include "../../include/nmp.h"

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int nmp_parse_type(const char *token, nmp_msg_type_t *out_type) {
    if (strcmp(token, "REGISTER") == 0) {
        *out_type = NMP_REGISTER;
        return 0;
    }
    if (strcmp(token, "STATUS") == 0) {
        *out_type = NMP_STATUS;
        return 0;
    }
    if (strcmp(token, "EVENT") == 0) {
        *out_type = NMP_EVENT;
        return 0;
    }
    if (strcmp(token, "ACK") == 0) {
        *out_type = NMP_ACK;
        return 0;
    }
    if (strcmp(token, "QUERY") == 0) {
        *out_type = NMP_QUERY;
        return 0;
    }
    if (strcmp(token, "RESPONSE") == 0) {
        *out_type = NMP_RESPONSE;
        return 0;
    }
    if (strcmp(token, "ERROR") == 0) {
        *out_type = NMP_ERROR;
        return 0;
    }

    return -1;
}

static int nmp_parse_id(const char *token, unsigned int *out_id) {
    char *end = NULL;
    unsigned long parsed = 0UL;

    if (token == NULL || token[0] == '\0') {
        return -1;
    }

    for (size_t i = 0; token[i] != '\0'; ++i) {
        if (!isdigit((unsigned char)token[i])) {
            return -1;
        }
    }

    parsed = strtoul(token, &end, 10);
    if (end == token || *end != '\0' || parsed > UINT_MAX) {
        return -1;
    }

    *out_id = (unsigned int)parsed;
    return 0;
}

int nmp_parse(const char *raw, nmp_message_t *out) {
    char buffer[512];
    char *first_sep = NULL;
    char *second_sep = NULL;
    char *third_sep = NULL;
    char *payload_start = NULL;
    size_t raw_len = 0U;
    size_t type_len = 0U;
    size_t node_len = 0U;
    size_t data_len = 0U;

    if (raw == NULL || out == NULL) {
        return -1;
    }

    raw_len = strlen(raw);
    if (raw_len == 0U || raw_len >= sizeof(buffer)) {
        return -1;
    }

    memset(out, 0, sizeof(*out));
    memcpy(buffer, raw, raw_len + 1U);

    first_sep = strchr(buffer, '|');
    if (first_sep == NULL) {
        return -1;
    }
    *first_sep = '\0';

    type_len = strlen(buffer);
    if (type_len == 0U || nmp_parse_type(buffer, &out->type) != 0) {
        return -1;
    }

    second_sep = strchr(first_sep + 1, '|');
    if (second_sep == NULL) {
        return -1;
    }
    *second_sep = '\0';

    if (nmp_parse_id(first_sep + 1, &out->id) != 0) {
        return -1;
    }
    if (strlen(first_sep + 1) >= sizeof(out->id_raw)) {
        return -1;
    }
    strcpy(out->id_raw, first_sep + 1);

    payload_start = second_sep + 1;
    third_sep = strchr(payload_start, '|');
    if (third_sep != NULL) {
        *third_sep = '\0';
        node_len = (size_t)(third_sep - payload_start);
        if (node_len == 0U || node_len >= sizeof(out->node_id)) {
            return -1;
        }
        memcpy(out->node_id, payload_start, node_len);
        out->node_id[node_len] = '\0';

        payload_start = third_sep + 1;
        data_len = strlen(payload_start);
        if (data_len >= sizeof(out->data)) {
            return -1;
        }
        if (data_len > 0U) {
            memcpy(out->data, payload_start, data_len);
            out->data[data_len] = '\0';
        }
    } else {
        node_len = strlen(payload_start);
        if (node_len == 0U || node_len >= sizeof(out->node_id)) {
            return -1;
        }
        memcpy(out->node_id, payload_start, node_len);
        out->node_id[node_len] = '\0';
    }

    return 0;
}
