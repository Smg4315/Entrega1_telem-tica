"""Cliente NMP real hacia server/. Desde la perspectiva de server/, este gateway
es simplemente "un cliente" más — server/ no sabe que existe un navegador detrás.

TODO: reemplazar el stub por sockets reales:
    import socket
    # TCP para REGISTER/EVENT/QUERY/RESPONSE/ERROR (docx, sección 9)
    # UDP (socket.SOCK_DGRAM) para STATUS
hasta que exista server/ escuchando en un puerto real, esto no puede probarse end-to-end.
"""
from .parser import NmpMessage


async def send_nmp(msg: NmpMessage) -> NmpMessage:
    raise NotImplementedError(
        "nmp.client.send_nmp: pendiente de conectar con server/ (ver TODO en este archivo)"
    )


def connect_nmp(host: str, port: int):
    raise NotImplementedError("nmp.client.connect_nmp: pendiente — requiere server/ real")
