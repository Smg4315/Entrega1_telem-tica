"""Reenvía al navegador (vía WebSocket) los mensajes NMP entrantes en tiempo real:
esto es lo que en client-web/ hoy simula mockData.mockProtocolLog con datos estáticos.
"""
from fastapi import WebSocket
from ..nmp.parser import NmpMessage

connected_clients: list[WebSocket] = []


async def broadcast_nmp_event(msg: NmpMessage) -> None:
    payload = {"type": msg.type, "id": msg.id, "nodeId": msg.node_id, "data": msg.data}
    for ws in list(connected_clients):
        await ws.send_json(payload)
