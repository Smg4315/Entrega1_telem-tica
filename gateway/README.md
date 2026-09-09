# gateway/ — Puente HTTP/WebSocket ↔ NMP (Python)

Traduce entre `client-web/` (navegador, solo HTTP/WebSocket) y `server/` (C, sockets NMP reales).
Es la única pieza además de `server/` que abre un socket real hacia el servidor, y la única
que habla LDAP en nombre del cliente web — un navegador tampoco puede hacer bind LDAP directo.

## Stack

FastAPI + uvicorn (HTTP + WebSocket nativo), `socket`/`socketserver` de la librería estándar
para el cliente NMP (TCP/UDP), `ldap3` para el bind LDAP.

## Estructura

- `app/nmp/parser.py` — mismo formato `TIPO|ID|NODE_ID|DATOS`, parseo/serialización en Python
- `app/nmp/client.py` — abre el socket TCP/UDP hacia `server/`, actúa como "cliente NMP" real
- `app/ws/live_feed.py` — reenvía STATUS/EVENT/ACK entrantes al navegador en tiempo real
- `app/http/auth_routes.py` — `POST /login`, delega en `ldap_bind`
- `app/http/query_routes.py` — `GET /nodos/{id}/historico` → `QUERY` NMP → `RESPONSE`
- `app/auth/ldap_bind.py` — única pieza que habla LDAP

## Ejecutar

```
pip install -r requirements.txt
uvicorn app.main:app --reload --port 4000
```

## Estado

Esqueleto ejecutable (levanta y responde), pero `nmp/client.py` y `auth/ldap_bind.py` tienen
la conexión real pendiente (TODO) hasta que `server/` y el LDAP existan.
