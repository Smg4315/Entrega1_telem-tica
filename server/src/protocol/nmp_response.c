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
            response->type = NMP_ERROR;
            strncpy(response->data, "INVALID_MESSAGE",
                    sizeof(response->data) - 1);
            break;

        default:
            return -1;
    }

    response->data[sizeof(response->data) - 1] = '\0';

    return 0;
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
    int written;

    if (message == NULL || out == NULL || out_len == 0) {
        return -1;
    }

    type = nmp_type_to_string(message->type);

    if (type == NULL) {
        return -1;
    }

    written = snprintf(
        out,
        out_len,
        "%s|%u|%s|%s",
        type,
        message->id,
        message->node_id,
        message->data
    );

    if (written < 0 || (size_t)written >= out_len) {
        return -1;
    }

    return 0;
}