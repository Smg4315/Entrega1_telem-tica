// URL del gateway (gateway/app/main.py). Se puede cambiar con VITE_GATEWAY_URL.
export const GATEWAY_URL: string = import.meta.env.VITE_GATEWAY_URL ?? "http://localhost:8011";

// Cada cuánto se consulta el estado de cada nodo.
export const POLL_INTERVAL_MS = 5000;

// Tiempo máximo de espera por consulta; menor que el intervalo de sondeo.
export const FETCH_TIMEOUT_MS = 4000;

// NMP no tiene un mensaje para listar nodos: los IDs a consultar se fijan aquí.
// label y location son texto descriptivo local; el servidor no los conoce.
export const KNOWN_NODES = [
  { id: "NODE01", label: "Servidor Web Principal", location: "Rack A — Sala 1" },
  { id: "NODE02", label: "Base de Datos Primaria", location: "Rack B — Sala 1" },
];
