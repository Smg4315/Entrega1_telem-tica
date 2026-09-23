/* Registro de nodos: valida REGISTER, rechaza nodos desconocidos (docx, sección 5). */
#include "../../include/node_registry.h"

#include <pthread.h>
#include <stdio.h>
#include <string.h>

#define MAX_NODES   64
#define NODE_ID_LEN 32

static char registered[MAX_NODES][NODE_ID_LEN];
static int node_count = 0;
static pthread_mutex_t registry_lock = PTHREAD_MUTEX_INITIALIZER;

/* Requiere registry_lock tomado. */
static int find_node(const char *node_id) {
    for (int i = 0; i < node_count; ++i) {
        if (strcmp(registered[i], node_id) == 0) {
            return i;
        }
    }
    return -1;
}

int is_node_registered(const char *node_id) {
    int found;

    if (node_id == NULL || node_id[0] == '\0') {
        return 0;
    }

    pthread_mutex_lock(&registry_lock);
    found = find_node(node_id) >= 0;
    pthread_mutex_unlock(&registry_lock);

    return found;
}

void register_node(const char *node_id) {
    if (node_id == NULL || node_id[0] == '\0' || strlen(node_id) >= NODE_ID_LEN) {
        return;
    }

    pthread_mutex_lock(&registry_lock);

    if (find_node(node_id) < 0) {
        if (node_count < MAX_NODES) {
            strcpy(registered[node_count++], node_id);
        } else {
            fprintf(stderr, "node_registry: lleno, no se registró %s\n", node_id);
        }
    }

    pthread_mutex_unlock(&registry_lock);
}
