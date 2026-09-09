# NetMonitor

Sistema distribuido de monitoreo y control de infraestructura — Telemática, Entrega 1.

Arquitectura de tres capas comunicadas por un protocolo propio (NMP) sobre sockets Berkeley, con un gateway que traduce hacia HTTP/WebSocket para el cliente web.

## Estado del proyecto

| Componente | Estado |
|---|---|
| `server/` | Sin implementar — stubs y TODOs |
| `gateway/` | Esqueleto ejecutable — stubs `NotImplementedError` en `nmp/client.py` y `auth/ldap_bind.py` |
| `client-web/` | Funcional (mock data) |
| `node-simulator/` | Sin implementar |
| `docs/protocolo-NMP.md` | Especificación redactada |

## Arquitectura

```
[Nodo real] --NMP/TCP+UDP--> [server/] <--NMP/TCP+UDP-- [gateway/] <--HTTP+WebSocket--> [client-web/]
                                              |
                                              +--LDAP bind--> [Servidor LDAP]
```

`server/` no distingue clientes: para él, `gateway/` es un cliente NMP más. El navegador no puede abrir sockets TCP/UDP crudos ni hablar LDAP directo — el gateway resuelve ambas limitaciones en nombre del cliente web.

## Componentes

| Carpeta | Lenguaje | Responsabilidad |
|---|---|---|
| `server/` | C | Servidor central NMP. Único componente con sockets Berkeley reales (TCP/UDP). |
| `gateway/` | Python (FastAPI + uvicorn) | Traduce HTTP/WebSocket ↔ NMP. Realiza el bind LDAP en nombre del cliente web. |
| `client-web/` | React + TypeScript + Vite + Tailwind v4 + Recharts | Cliente administrativo real del proyecto. Solo conoce HTTP/WebSocket. |
| `node-simulator/` | Por definir | Simula nodos NMP para pruebas sin hardware real. |
| `docs/` | — | Especificación del protocolo NMP y diagramas. |
| `logs/` | — | Salida en tiempo de ejecución (gitignored). |

## Protocolo NMP

Formato de mensaje: `TIPO|ID|NODE_ID|DATOS`

Especificación completa, tipos de mensaje y máquinas de estado en [`docs/protocolo-NMP.md`](docs/protocolo-NMP.md).

## Ejecución

### server (C)

```bash
cd server
make
./netmonitor-server
```

### gateway (Python)

```bash
cd gateway
python -m venv .venv
source .venv/bin/activate   # Windows: .venv\Scripts\activate
pip install -r requirements.txt
uvicorn main:app --reload
```

### client-web (React)

```bash
cd client-web
npm install
npm run dev
```

## Decisiones de diseño

- Servidor 100 % C, sockets Berkeley puros — sin frameworks HTTP.
- Gateway en Python/FastAPI — capa de traducción NMP ↔ HTTP/WebSocket, no lógica de negocio propia.
- `client-web` es el cliente administrativo real, no un prototipo.

## Estructura del repositorio

```
netmonitor/
├── server/            # C — protocolo NMP sobre sockets Berkeley
├── gateway/            # Python — FastAPI, traducción NMP/LDAP
├── client-web/         # React + TS — dashboard administrativo
├── node-simulator/     # Simulador de nodos (pendiente)
└── docs/
    └── protocolo-NMP.md
```
