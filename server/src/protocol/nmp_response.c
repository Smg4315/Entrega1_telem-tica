#include "../../include/nmp_response.h"

#include <stdio.h>
#include <string.h>

int nmp_build_response(
    const nmp_message_t *request,
    nmp_message_t *response
) {
    if (request == NULL || response == NULL) {
        return -1;
    }

    memset(response, 0, sizeof(*response));

    response->id = request->id;
    strncpy(response->id_raw, request->id_raw,
            sizeof(response->id_raw) - 1);
    strncpy(response->node_id, request->node_id,
            sizeof(response->node_id) - 1);
    response->node_id[sizeof(response->node_id) - 1] = '\0';

    switch (request->type) {

        case NMP_REGISTER:
            response->type = NMP_ACK;
            strncpy(response->data, "REGISTER",
                    sizeof(response->data) - 1);
            break;

        case NMP_STATUS:
            response->type = NMP_ACK;
            strncpy(response->data, "STATUS",
                    sizeof(response->data) - 1);
            break;

        case NMP_EVENT:
            response->type = NMP_ACK;
            strncpy(response->data, "EVENT",
                    sizeof(response->data) - 1);
            break;

        case NMP_QUERY:
            response->type = NMP_RESPONSE;
            strncpy(response->data, request->data,
                    sizeof(response->data) - 1);
            break;

        case NMP_ACK:
        case NMP_RESPONSE:
        case NMP_ERROR:
            nmp_build_error(request, "INVALID_MESSAGE", response);
            break;

        default:
            return -1;
    }

    response->data[sizeof(response->data) - 1] = '\0';

    return 0;
}

void nmp_build_error(
    const nmp_message_t *request,
    const char *code,
    nmp_message_t *response
) {
    if (response == NULL || code == NULL) {
        return;
    }

    memset(response, 0, sizeof(*response));
    response->type = NMP_ERROR;

    if (request != NULL) {
        response->id = request->id;
        strncpy(response->id_raw, request->id_raw,
                sizeof(response->id_raw) - 1);
        strncpy(response->node_id, request->node_id,
                sizeof(response->node_id) - 1);
    }

    strncpy(response->data, code, sizeof(response->data) - 1);
}

static const char *nmp_type_to_string(nmp_msg_type_t type) {
    switch (type) {
        case NMP_REGISTER: return "REGISTER";
        case NMP_STATUS:   return "STATUS";
        case NMP_EVENT:    return "EVENT";
        case NMP_ACK:      return "ACK";
        case NMP_QUERY:    return "QUERY";
        case NMP_RESPONSE: return "RESPONSE";
        case NMP_ERROR:    return "ERROR";
        default:           return NULL;
    }
}

int nmp_serialize_response(
    const nmp_message_t *message,
    char *out,
    size_t out_len
) {
    const char *type;
    char id_buf[16];
    const char *id;
    int written;

    if (message == NULL || out == NULL || out_len == 0) {
        return -1;
    }

    type = nmp_type_to_string(message->type);

    if (type == NULL) {
        return -1;
    }

    if (message->id_raw[0] != '\0') {
        id = message->id_raw;
    } else {
        snprintf(id_buf, sizeof(id_buf), "%u", message->id);
        id = id_buf;
    }

    if (message->type == NMP_ACK || message->type == NMP_ERROR) {
        /* Spec: ACK|001|REGISTER|NODE01, ERROR|101|UNKNOWN_NODE|NODE99 */
        written = snprintf(out, out_len, "%s|%s|%s|%s",
                           type, id, message->data, message->node_id);
    } else {
        written = snprintf(out, out_len, "%s|%s|%s|%s",
                           type, id, message->node_id, message->data);
    }

    if (written < 0 || (size_t)written >= out_len) {
        return -1;
    }

    return 0;
}