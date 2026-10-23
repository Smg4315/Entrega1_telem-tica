# node-simulator/ — Simulador de nodos NMP

Simula uno o varios nodos que hablan NMP real contra `server/`, para demostrar el flujo
completo sin hardware físico. Solo usa la biblioteca estándar de Python.

Cada nodo simulado:

- se registra por TCP (`REGISTER`) y reintenta hasta que el servidor responda;
- envía `STATUS` por UDP cada `INTERVAL` segundos, con CPU, MEM, TEMP y BAT que varían de forma gradual;
- envía `EVENT` por TCP al cruzar un umbral: `HIGH_TEMP` (TEMP ≥ 80) o `LOW_BAT` (BAT ≤ 15);
- no pierde `EVENT` si se queda sin red: los guarda en una cola (hasta 100 por nodo) y los reenvía en orden, con su ID original, hasta recibir `ACK`;
- se vuelve a registrar si el servidor responde `UNKNOWN_NODE` (por ejemplo, tras reiniciarse).

## Uso

Con Docker Compose arranca solo, como el servicio `node-simulator`. Fuera de Docker:

```bash
python simulator.py                          # NODE01 y NODE02 contra localhost:8080
NODES=NODE01,NODE03 INTERVAL=2 python simulator.py
```

| Variable | Por defecto | Descripción |
|---|---|---|
| `NMP_HOST` | `localhost` | Host del servidor NMP |
| `NMP_PORT` | `8080` | Puerto TCP/UDP del servidor |
| `NODES` | `NODE01,NODE02` | Nodos a simular, separados por coma |
| `INTERVAL` | `3` | Segundos entre `STATUS` |

## Demostrar una desconexión

```bash
docker compose stop node-simulator    # tras 15 s sin STATUS, el servidor marca los nodos DESCONECTADO
docker compose start node-simulator   # vuelven a estar activos en el siguiente STATUS
```

Los 15 s vienen de `NMP_NODE_TIMEOUT` en `docker-compose.yml`; fuera de Docker el servidor usa 60 s.

Para simular un corte de red sin detener el proceso (así se ve la cola de `EVENT` en acción):

```bash
docker network disconnect entrega1_telem-tica_default entrega1_telem-tica-node-simulator-1
docker network connect --alias node-simulator entrega1_telem-tica_default entrega1_telem-tica-node-simulator-1
```

El nombre de la red y del contenedor depende de la carpeta del proyecto: compruébalos con `docker network ls` y `docker compose ps`.
