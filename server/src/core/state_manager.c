/* Estado actual por nodo (docx, sección 6 — Máquinas de Estado).
 * Solo el último valor de cada métrica; el histórico queda fuera de Fase 2. */
#include "../../include/state_manager.h"

#include <pthread.h>
#include <stdio.h>
#include <string.h>

#define MAX_NODES     64
#define NODE_ID_LEN   32
#define MAX_METRICS   16
#define KEY_LEN       32
#define VALUE_LEN     32
#define STATE_STR_LEN 256

typedef struct {
    char key[KEY_LEN];
    char value[VALUE_LEN];
} metric_t;

typedef struct {
    char     node_id[NODE_ID_LEN];
    metric_t metrics[MAX_METRICS];
    int      metric_count;
} node_state_t;

static node_state_t states[MAX_NODES];
static int state_count = 0;
static pthread_mutex_t state_lock = PTHREAD_MUTEX_INITIALIZER;

/* Requiere state_lock tomado. */
static node_state_t *find_state(const char *node_id) {
    for (int i = 0; i < state_count; ++i) {
        if (strcmp(states[i].node_id, node_id) == 0) {
            return &states[i];
        }
    }
    return NULL;
}

/* Requiere state_lock tomado. */
static void set_metric(node_state_t *st, const char *key, size_t key_len,
                       const char *value, size_t value_len) {
    if (key_len == 0 || key_len >= KEY_LEN || value_len >= VALUE_LEN) {
        return;
    }

    for (int i = 0; i < st->metric_count; ++i) {
        if (strlen(st->metrics[i].key) == key_len &&
            strncmp(st->metrics[i].key, key, key_len) == 0) {
            memcpy(st->metrics[i].value, value, value_len);
            st->metrics[i].value[value_len] = '\0';
            return;
        }
    }

    if (st->metric_count >= MAX_METRICS) {
        return;
    }

    metric_t *m = &st->metrics[st->metric_count++];
    memcpy(m->key, key, key_len);
    m->key[key_len] = '\0';
    memcpy(m->value, value, value_len);
    m->value[value_len] = '\0';
}

void update_node_state(const char *node_id, const char *data) {
    if (node_id == NULL || node_id[0] == '\0' || strlen(node_id) >= NODE_ID_LEN) {
        return;
    }

    pthread_mutex_lock(&state_lock);

    node_state_t *st = find_state(node_id);
    if (st == NULL) {
        if (state_count >= MAX_NODES) {
            pthread_mutex_unlock(&state_lock);
            fprintf(stderr, "state_manager: lleno, no se guardó estado de %s\n", node_id);
            return;
        }
        st = &states[state_count++];
        memset(st, 0, sizeof(*st));
        strcpy(st->node_id, node_id);
    }

    /* Recorre data token a token (separador '|') y fusiona los KEY=VALUE. */
    const char *p = (data != NULL) ? data : "";
    while (*p != '\0') {
        const char *end = strchr(p, '|');
        size_t tok_len = (end != NULL) ? (size_t)(end - p) : strlen(p);
        const char *eq = memchr(p, '=', tok_len);

        if (eq != NULL) {
            set_metric(st, p, (size_t)(eq - p), eq + 1, tok_len - (size_t)(eq - p) - 1);
        }

        p += tok_len;
        if (*p == '|') {
            ++p;
        }
    }

    pthread_mutex_unlock(&state_lock);
}

const char *get_node_state(const char *node_id) {
    static _Thread_local char out[STATE_STR_LEN];

    if (node_id == NULL) {
        return NULL;
    }

    pthread_mutex_lock(&state_lock);

    node_state_t *st = find_state(node_id);
    if (st == NULL) {
        pthread_mutex_unlock(&state_lock);
        return NULL;
    }

    size_t used = 0;
    out[0] = '\0';
    for (int i = 0; i < st->metric_count; ++i) {
        int n = snprintf(out + used, sizeof(out) - used, "%s%s=%s",
                         (i > 0) ? "|" : "", st->metrics[i].key, st->metrics[i].value);
        if (n < 0 || (size_t)n >= sizeof(out) - used) {
            break; /* no cabe: se devuelve lo acumulado hasta aquí */
        }
        used += (size_t)n;
    }
    out[used] = '\0';

    pthread_mutex_unlock(&state_lock);
    return out;
}
