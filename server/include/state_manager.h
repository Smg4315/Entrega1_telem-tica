#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

/* Estado actual por nodo (sin histórico — fuera del alcance de Fase 2).
 * El estado es un conjunto clave=valor: CPU=45|MEM=62|TEMP=38|BAT=87. */

/* Fusiona los pares KEY=VALUE de data en el estado del nodo (los tokens sin
 * '=' se ignoran, ej. el nombre del evento HIGH_TEMP en un EVENT). */
void update_node_state(const char *node_id, const char *data);

/* Retorna el estado actual como "K=V|K=V...", "" si el nodo no tiene métricas
 * o NULL si nunca se registró estado. El buffer es propio de cada hilo y
 * válido hasta la siguiente llamada desde ese mismo hilo. */
const char *get_node_state(const char *node_id);

#endif
