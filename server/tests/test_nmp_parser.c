#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../include/nmp.h"

static void test_valid_register(void) {
    nmp_message_t msg;
    int rc = nmp_parse("REGISTER|001|NODE01", &msg);
    assert(rc == 0);
    assert(msg.type == NMP_REGISTER);
    assert(msg.id == 1U);
    assert(strcmp(msg.node_id, "NODE01") == 0);
    assert(strcmp(msg.data, "") == 0);
}

static void test_valid_status(void) {
    nmp_message_t msg;
    int rc = nmp_parse("STATUS|002|NODE01|CPU=45|MEM=62|TEMP=38|BAT=87", &msg);
    assert(rc == 0);
    assert(msg.type == NMP_STATUS);
    assert(msg.id == 2U);
    assert(strcmp(msg.node_id, "NODE01") == 0);
    assert(strcmp(msg.data, "CPU=45|MEM=62|TEMP=38|BAT=87") == 0);
    assert(strstr(msg.data, "MEM=62|TEMP=38|BAT=87") != NULL);
}

static void test_valid_status_without_payload_documented(void) {
    nmp_message_t msg;
    int rc = nmp_parse("STATUS|002|NODE01", &msg);
    assert(rc == 0);
    assert(msg.type == NMP_STATUS);
    assert(msg.id == 2U);
    assert(strcmp(msg.node_id, "NODE01") == 0);
    assert(strcmp(msg.data, "") == 0);
}

static void test_valid_status_with_empty_payload_documented(void) {
    nmp_message_t msg;
    int rc = nmp_parse("STATUS|002|NODE01|", &msg);
    assert(rc == 0);
    assert(msg.type == NMP_STATUS);
    assert(msg.id == 2U);
    assert(strcmp(msg.node_id, "NODE01") == 0);
    assert(strcmp(msg.data, "") == 0);
}

static void test_valid_status_with_multikey_payload_documented(void) {
    nmp_message_t msg;
    int rc = nmp_parse("STATUS|002|NODE01|CPU=45|MEM=62", &msg);
    assert(rc == 0);
    assert(msg.type == NMP_STATUS);
    assert(msg.id == 2U);
    assert(strcmp(msg.node_id, "NODE01") == 0);
    assert(strcmp(msg.data, "CPU=45|MEM=62") == 0);
}

static void test_valid_status_with_double_pipe_payload_documented(void) {
    nmp_message_t msg;
    int rc = nmp_parse("STATUS|002|NODE01|CPU=45||MEM=62", &msg);
    assert(rc == 0);
    assert(msg.type == NMP_STATUS);
    assert(msg.id == 2U);
    assert(strcmp(msg.node_id, "NODE01") == 0);
    assert(strcmp(msg.data, "CPU=45||MEM=62") == 0);
}

static void test_valid_event(void) {
    nmp_message_t msg;
    int rc = nmp_parse("EVENT|003|NODE01|HIGH_TEMP|TEMP=92", &msg);
    assert(rc == 0);
    assert(msg.type == NMP_EVENT);
    assert(msg.id == 3U);
    assert(strcmp(msg.node_id, "NODE01") == 0);
    assert(strcmp(msg.data, "HIGH_TEMP|TEMP=92") == 0);
}

static void test_valid_ack(void) {
    nmp_message_t msg;
    int rc = nmp_parse("ACK|001|REGISTER|NODE01", &msg);
    assert(rc == 0);
    assert(msg.type == NMP_ACK);
    assert(msg.id == 1U);
    assert(strcmp(msg.node_id, "REGISTER") == 0);
    assert(strcmp(msg.data, "NODE01") == 0);
}

static void test_valid_error(void) {
    nmp_message_t msg;
    int rc = nmp_parse("ERROR|101|UNKNOWN_NODE|NODE99", &msg);
    assert(rc == 0);
    assert(msg.type == NMP_ERROR);
    assert(msg.id == 101U);
    assert(strcmp(msg.node_id, "UNKNOWN_NODE") == 0);
    assert(strcmp(msg.data, "NODE99") == 0);
}

static void test_valid_query(void) {
    nmp_message_t msg;
    int rc = nmp_parse("QUERY|101|NODE01|CURRENT", &msg);
    assert(rc == 0);
    assert(msg.type == NMP_QUERY);
    assert(msg.id == 101U);
    assert(strcmp(msg.node_id, "NODE01") == 0);
    assert(strcmp(msg.data, "CURRENT") == 0);
}

static void test_valid_response(void) {
    nmp_message_t msg;
    int rc = nmp_parse("RESPONSE|101|NODE01|CURRENT|CPU=45|MEM=62|TEMP=38|BAT=87", &msg);
    assert(rc == 0);
    assert(msg.type == NMP_RESPONSE);
    assert(msg.id == 101U);
    assert(strcmp(msg.node_id, "NODE01") == 0);
    assert(strcmp(msg.data, "CURRENT|CPU=45|MEM=62|TEMP=38|BAT=87") == 0);
}

static void test_invalid_unknown_type(void) {
    nmp_message_t msg;
    assert(nmp_parse("UNKNOWN|001|NODE01|DATA", &msg) != 0);
}

static void test_null_input(void) {
    nmp_message_t msg;
    assert(nmp_parse(NULL, &msg) != 0);
    assert(nmp_parse("REGISTER|001|NODE01", NULL) != 0);
}

static void test_empty_input(void) {
    nmp_message_t msg;
    assert(nmp_parse("", &msg) != 0);
}

static void test_incomplete_message(void) {
    nmp_message_t msg;
    assert(nmp_parse("STATUS|001", &msg) != 0);
}

int main(void) {
    test_valid_register();
    test_valid_status();
    test_valid_status_without_payload_documented();
    test_valid_status_with_empty_payload_documented();
    test_valid_status_with_multikey_payload_documented();
    test_valid_status_with_double_pipe_payload_documented();
    test_valid_event();
    test_valid_ack();
    test_valid_error();
    test_valid_query();
    test_valid_response();
    test_invalid_unknown_type();
    test_null_input();
    test_empty_input();
    test_incomplete_message();

    puts("All NMP parser tests passed.");
    return 0;
}
