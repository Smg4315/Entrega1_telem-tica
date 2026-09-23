#ifndef NODE_REGISTRY_H
#define NODE_REGISTRY_H

/* Registro en memoria de nodos (docx, sección 5). Seguro entre hilos TCP/UDP. */

/* Retorna 1 si node_id ya envió REGISTER, 0 si no. */
int is_node_registered(const char *node_id);

/* Registra node_id; idempotente. Si el registro está lleno, lo informa por stderr. */
void register_node(const char *node_id);

#endif
