"""GET /nodos/{node_id}/historico — traduce la petición HTTP del front en un QUERY NMP real.

La ruta conserva el nombre "historico", pero en Fase 3 el servidor solo
soporta QUERY ... CURRENT (estado actual).
"""
import itertools
import re

from fastapi import APIRouter
from fastapi.responses import JSONResponse

from ..nmp.client import connect_nmp, send_nmp
from ..nmp.parser import NmpMessage, parse_nmp, serialize_nmp

router = APIRouter()

# ID numérico por mensaje (el servidor solo acepta dígitos).
_next_id = itertools.count(1)

# NODE_ID: hasta 31 caracteres, sin '|' ni saltos de línea, para que el valor
# de la URL no pueda alterar la estructura del mensaje NMP.
_NODE_ID_RE = re.compile(r"[A-Za-z0-9_.-]{1,31}")

# Código HTTP por código de error NMP; cualquier otro se informa como 400.
_HTTP_STATUS = {"UNKNOWN_NODE": 404}


def _error(status: int, code: str, node_id: str) -> JSONResponse:
    return JSONResponse(status_code=status, content={"error": code, "nodeId": node_id})


# def (no async): los sockets son bloqueantes y FastAPI ejecuta esta función
# en un hilo aparte, sin detener a los demás clientes del gateway.
@router.get("/nodos/{node_id}/historico")
def historico(node_id: str):
    if not _NODE_ID_RE.fullmatch(node_id):
        return _error(400, "INVALID_PARAMETER", node_id)

    request = serialize_nmp(
        NmpMessage(type="QUERY", id=str(next(_next_id)), node_id=node_id, data="CURRENT")
    )

    try:
        with connect_nmp() as conn:
            raw = send_nmp(conn, request)
    except OSError:
        # Incluye conexión rechazada, timeout, fallo de DNS y cierre prematuro.
        return _error(502, "SERVER_UNREACHABLE", node_id)

    try:
        reply = parse_nmp(raw)
    except ValueError:
        return _error(502, "INVALID_REPLY", node_id)

    if reply.type == "ERROR":
        # reply.data es el código: UNKNOWN_NODE, INVALID_MESSAGE, INVALID_PARAMETER...
        return _error(_HTTP_STATUS.get(reply.data, 400), reply.data, node_id)

    if reply.type != "RESPONSE":
        return _error(502, "INVALID_REPLY", node_id)

    # DATOS: CURRENT|CPU=45|MEM=62|...[|DESCONECTADO]
    metrics = {}
    connected = True
    for token in reply.data.split("|"):
        if token == "DESCONECTADO":
            connected = False
        elif "=" in token:
            key, value = token.split("=", 1)
            metrics[key] = value

    return {
        "nodeId": reply.node_id,
        "status": "active" if connected else "disconnected",
        "metrics": metrics,
    }
