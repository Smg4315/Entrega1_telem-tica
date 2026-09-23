/* Registro de nodos: valida REGISTER, rechaza nodos desconocidos (docx, sección 5).
 * Los datos viven en memoria compartida (mmap MAP_SHARED) para que el padre,
 * el hilo UDP y los hijos TCP creados con fork() vean el mismo registro. */
#include "../../include/node_registry.h"

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

#define MAX_NODES   64
#define NODE_ID_LEN 32

typedef struct {
    pthread_mutex_t lock; /* PTHREAD_PROCESS_SHARED */
    int  count;
    char ids[MAX_NODES][NODE_ID_LEN];
} registry_t;

static registry_t *reg = NULL;
static pthread_once_t reg_once = PTHREAD_ONCE_INIT;

static void registry_create(void) {
    void *mem = mmap(NULL, sizeof(registry_t), PROT_READ | PROT_WRITE,
                     MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (mem == MAP_FAILED) {
        perror("node_registry: mmap");
        return;
    }

    registry_t *r = mem;
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&r->lock, &attr);
    pthread_mutexattr_destroy(&attr);
    r->count = 0;

    reg = r;
}

int node_registry_init(void) {
    pthread_once(&reg_once, registry_create);
    return (reg != NULL) ? 0 : -1;
}

/* Requiere reg->lock tomado. */
static int find_node(const char *node_id) {
    for (int i = 0; i < reg->count; ++i) {
        if (strcmp(reg->ids[i], node_id) == 0) {
            return i;
        }
    }
    return -1;
}

int is_node_registered(const char *node_id) {
    int found;

    if (node_id == NULL || node_id[0] == '\0' || node_registry_init() != 0) {
        return 0;
    }

    pthread_mutex_lock(&reg->lock);
    found = find_node(node_id) >= 0;
    pthread_mutex_unlock(&reg->lock);

    return found;
}

void register_node(const char *node_id) {
    if (node_id == NULL || node_id[0] == '\0' || strlen(node_id) >= NODE_ID_LEN ||
        node_registry_init() != 0) {
        return;
    }

    pthread_mutex_lock(&reg->lock);

    if (find_node(node_id) < 0) {
        if (reg->count < MAX_NODES) {
            strcpy(reg->ids[reg->count++], node_id);
        } else {
            fprintf(stderr, "node_registry: lleno, no se registró %s\n", node_id);
        }
    }

    pthread_mutex_unlock(&reg->lock);
}
