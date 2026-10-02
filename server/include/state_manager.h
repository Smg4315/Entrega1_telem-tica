#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

/* Estado actual por nodo (sin histórico — fuera del alcance de Fase 2).
 * El estado es un conjunto clave=valor: CPU=45|MEM=62|TEMP=38|BAT=87.
 * Seguro entre hilos y entre procesos creados con fork(). */

/* Crea la memoria compartida. Llamar desde main() antes de fork()/hilos;
 * idempotente (las demás funciones también la invocan). 0 ok, -1 error. */
int state_manager_init(void);

/* Fusiona los pares KEY=VALUE de data en el estado del nodo (los tokens sin
 * '=' se ignoran, ej. el nombre del evento HIGH_TEMP en un EVENT). */
void update_node_state(const char *node_id, const char *data);

/* Retorna el estado actual como "K=V|K=V...", "" si el nodo no tiene métricas
 * o NULL si nunca se registró estado. Si el nodo no envía nada dentro de la
 * ventana NMP_NODE_TIMEOUT (segundos, 60 por defecto) se agrega "DESCONECTADO"
 * al final. El buffer es propio de cada hilo y
 * válido hasta la siguiente llamada desde ese mismo hilo. */
const char *get_node_state(const char *node_id);

#endif
