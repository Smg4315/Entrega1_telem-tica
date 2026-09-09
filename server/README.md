# server/ — Servidor central NMP (C, Berkeley Sockets)

Único componente que abre sockets TCP/UDP reales: implementación
exclusivamente en C con la API de Sockets Berkeley (sin frameworks HTTP).

## Estructura

- `src/net/` — `tcp_server.c` (REGISTER, EVENT, QUERY, RESPONSE, ERROR), `udp_server.c` (STATUS)
- `src/protocol/` — parsing/serialización `TIPO|ID|NODE_ID|DATOS`, testeable sin sockets reales
- `src/core/` — registro de nodos, estado actual, histórico
- `src/auth/` — cliente LDAP (bind de usuarios administrativos, incluido el que abre `gateway/`)
- `src/dns/` — resolución del nombre del servidor, reintento sin caer el servicio
- `src/log/` — persistencia de solicitudes/respuestas
- `config/` — puertos, endpoint LDAP, nombre DNS
- `tests/` — pruebas del parser NMP y la máquina de estados

## Estado

Pendiente de implementar. 