"""Arranque del gateway: expone HTTP (login, consultas históricas) y WebSocket
(feed en vivo de mensajes NMP) para client-web/.
"""
from fastapi import FastAPI, WebSocket, WebSocketDisconnect
from .http.auth_routes import router as auth_router
from .http.query_routes import router as query_router
from .ws.live_feed import connected_clients

app = FastAPI(title="NetMonitor Gateway")
app.include_router(auth_router)
app.include_router(query_router)


@app.websocket("/ws/live")
async def live_feed_endpoint(websocket: WebSocket):
    await websocket.accept()
    connected_clients.append(websocket)
    # TODO: aquí se conecta lo que llegue de server/ vía nmp.client, y se reenvía con
    # ws.live_feed.broadcast_nmp_event a cada cliente conectado.
    try:
        await websocket.send_json({"type": "INFO", "data": "conectado al gateway NetMonitor"})
        while True:
            await websocket.receive_text()  # mantiene la conexión viva
    except WebSocketDisconnect:
        connected_clients.remove(websocket)
