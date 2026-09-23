#include "../include/nmp_response.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_register(void) {
    nmp_message_t request = {
        .type = NMP_REGISTER,
        .id = 1
    };

    strcpy(request.node_id, "NODE01");

    nmp_message_t response;
    char buffer[512];

    assert(nmp_build_response(&request, &response) == 0);
    assert(response.type == NMP_ACK);
    assert(response.id == 1);
    assert(strcmp(response.node_id, "NODE01") == 0);
    assert(strcmp(response.data, "REGISTER") == 0);

    assert(nmp_serialize_response(&response, buffer, sizeof(buffer)) == 0);
    assert(strcmp(buffer, "ACK|1|NODE01|REGISTER") == 0);
}

static void test_status(void) {
    nmp_message_t request = {
        .type = NMP_STATUS,
        .id = 2
    };

    strcpy(request.node_id, "NODE01");
    strcpy(request.data, "CPU=45|MEM=62");

    nmp_message_t response;

    assert(nmp_build_response(&request, &response) == 0);
    assert(response.type == NMP_ACK);
    assert(response.id == 2);
    assert(strcmp(response.node_id, "NODE01") == 0);
    assert(strcmp(response.data, "STATUS") == 0);
}

static void test_event(void) {
    nmp_message_t request = {
        .type = NMP_EVENT,
        .id = 3
    };

    strcpy(request.node_id, "NODE01");
    strcpy(request.data, "HIGH_TEMP|TEMP=92");

    nmp_message_t response;

    assert(nmp_build_response(&request, &response) == 0);
    assert(response.type == NMP_ACK);
    assert(response.id == 3);
    assert(strcmp(response.data, "EVENT") == 0);
}

static void test_query(void) {
    nmp_message_t request = {
        .type = NMP_QUERY,
        .id = 101
    };

    strcpy(request.node_id, "NODE01");
    strcpy(request.data, "CURRENT");

    nmp_message_t response;
    char buffer[512];

    assert(nmp_build_response(&request, &response) == 0);
    assert(response.type == NMP_RESPONSE);
    assert(response.id == 101);
    assert(strcmp(response.node_id, "NODE01") == 0);
    assert(strcmp(response.data, "CURRENT") == 0);

    assert(nmp_serialize_response(&response, buffer, sizeof(buffer)) == 0);
    assert(strcmp(buffer, "RESPONSE|101|NODE01|CURRENT") == 0);
}

static void test_invalid_message(void) {
    nmp_message_t request = {
        .type = NMP_ACK,
        .id = 50
    };

    strcpy(request.node_id, "NODE01");

    nmp_message_t response;

    assert(nmp_build_response(&request, &response) == 0);
    assert(response.type == NMP_ERROR);
    assert(strcmp(response.data, "INVALID_MESSAGE") == 0);
}

static void test_invalid_arguments(void) {
    nmp_message_t request = {0};
    nmp_message_t response = {0};

    assert(nmp_build_response(NULL, &response) == -1);
    assert(nmp_build_response(&request, NULL) == -1);
}

int main(void) {
    test_register();
    test_status();
    test_event();
    test_query();
    test_invalid_message();
    test_invalid_arguments();

    printf("Todos los tests de nmp_response pasaron correctamente.\n");

    return 0;
}