#ifndef NODE_REGISTRY_H
#define NODE_REGISTRY_H

/* Registro en memoria de nodos (docx, sección 5). Seguro entre hilos y entre
 * procesos creados con fork(). */

/* Crea la memoria compartida. Llamar desde main() antes de fork()/hilos;
 * idempotente (las demás funciones también la invocan). 0 ok, -1 error. */
int node_registry_init(void);

/* Retorna 1 si node_id ya envió REGISTER, 0 si no. */
int is_node_registered(const char *node_id);

/* Registra node_id; idempotente. Si el registro está lleno, lo informa por stderr. */
void register_node(const char *node_id);

#endif
