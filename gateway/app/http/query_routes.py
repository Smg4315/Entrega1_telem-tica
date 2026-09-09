"""GET /nodos/{node_id}/historico — traduce la petición HTTP del front en un QUERY NMP real."""
from fastapi import APIRouter, HTTPException

router = APIRouter()


@router.get("/nodos/{node_id}/historico")
async def historico(node_id: str):
    # TODO: construir NmpMessage QUERY|<id>|<node_id>|CURRENT (o HISTORY), enviar con
    # nmp.client.send_nmp, esperar RESPONSE y traducirlo a JSON para el front.
    raise HTTPException(status_code=501, detail="pendiente: requiere server/ real")
