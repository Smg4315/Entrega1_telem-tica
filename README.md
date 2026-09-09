# NetMonitor

Sistema distribuido de monitoreo de infraestructura — Telemática, Fase 1.

## Componentes

| Carpeta | Lenguaje | Responsabilidad |
|---|---|---|
| `server/` | C | Servidor central NMP. Único componente con sockets Berkeley reales (TCP/UDP). |
| `gateway/` | Python (FastAPI) | Traduce HTTP/WebSocket (navegador) ↔ NMP (server/). Habla LDAP en nombre del cliente web. |
| `client-web/` | React/TypeScript | **Cliente administrativo real** del proyecto (no mockup). Solo conoce HTTP/WebSocket, nunca NMP ni LDAP directo. |
| `node-simulator/` | (a definir) | Simula nodos NMP para pruebas sin hardware real. |
| `docs/` | — | Especificación NMP y diagramas. |
| `logs/` | — | Salida en tiempo de ejecución (gitignored). |

## Flujo de comunicación

```
[Nodo real]  --NMP/TCP+UDP-->  [server/]  <--NMP/TCP+UDP--  [gateway/]  <--HTTP+WebSocket-->  [client-web/ en el navegador]
                                              |
                                              +--LDAP bind--> [Servidor LDAP]
```

`server/` no sabe que existe un navegador: para él, `gateway/` es simplemente otro cliente NMP.
