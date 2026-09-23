#include "../include/nmp_response.h"
#include "../include/node_registry.h"
#include "../include/state_manager.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_register(void) {
    nmp_message_t request = {
        .type = NMP_REGISTER,
        .id = 1,
        .id_raw = "001"
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
    assert(strcmp(buffer, "ACK|001|REGISTER|NODE01") == 0);
}

static void test_status(void) {
    nmp_message_t request = {
        .type = NMP_STATUS,
        .id = 2,
        .id_raw = "002"
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
        .id = 3,
        .id_raw = "003"
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
        .id = 101,
        .id_raw = "101"
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

static void test_query_with_state(void) {
    nmp_message_t request;
    nmp_message_t response;
    char buffer[512];

    update_node_state("NODE02", "CPU=45|MEM=62|TEMP=38|BAT=87");
    update_node_state("NODE02", "HIGH_TEMP|TEMP=92");

    assert(nmp_parse("QUERY|101|NODE02|CURRENT", &request) == 0);
    assert(nmp_build_response(&request, &response) == 0);

    assert(nmp_serialize_response(&response, buffer, sizeof(buffer)) == 0);
    assert(strcmp(buffer, "RESPONSE|101|NODE02|CURRENT|CPU=45|MEM=62|TEMP=92|BAT=87") == 0);
}

static void test_query_invalid_parameter(void) {
    nmp_message_t request;
    nmp_message_t response;
    char buffer[512];

    assert(nmp_parse("QUERY|102|NODE02|HISTORY", &request) == 0);
    assert(nmp_build_response(&request, &response) == 0);

    assert(nmp_serialize_response(&response, buffer, sizeof(buffer)) == 0);
    assert(strcmp(buffer, "ERROR|102|INVALID_PARAMETER|NODE02") == 0);
}

static void test_node_registry(void) {
    assert(!is_node_registered("NODE03"));
    register_node("NODE03");
    register_node("NODE03");
    assert(is_node_registered("NODE03"));
    assert(!is_node_registered("NODE99"));
}

static void test_invalid_message(void) {
    nmp_message_t request = {
        .type = NMP_ACK,
        .id = 50,
        .id_raw = "050"
    };

    strcpy(request.node_id, "NODE01");

    nmp_message_t response;
    char buffer[512];

    assert(nmp_build_response(&request, &response) == 0);
    assert(response.type == NMP_ERROR);
    assert(strcmp(response.data, "INVALID_MESSAGE") == 0);

    assert(nmp_serialize_response(&response, buffer, sizeof(buffer)) == 0);
    assert(strcmp(buffer, "ERROR|050|INVALID_MESSAGE|NODE01") == 0);
}

static void test_invalid_format_error(void) {
    nmp_message_t response;
    char buffer[512];

    nmp_build_error(NULL, "INVALID_FORMAT", &response);

    assert(nmp_serialize_response(&response, buffer, sizeof(buffer)) == 0);
    assert(strcmp(buffer, "ERROR|0|INVALID_FORMAT|") == 0);
}

static void test_parse_to_ack_preserves_id(void) {
    nmp_message_t request;
    nmp_message_t response;
    char buffer[512];

    assert(nmp_parse("STATUS|002|NODE01|CPU=45|MEM=62", &request) == 0);
    assert(nmp_build_response(&request, &response) == 0);

    assert(nmp_serialize_response(&response, buffer, sizeof(buffer)) == 0);
    assert(strcmp(buffer, "ACK|002|STATUS|NODE01") == 0);
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
    test_query_with_state();
    test_query_invalid_parameter();
    test_node_registry();
    test_invalid_message();
    test_invalid_format_error();
    test_parse_to_ack_preserves_id();
    test_invalid_arguments();

    printf("Todos los tests de nmp_response pasaron correctamente.\n");

    return 0;
}