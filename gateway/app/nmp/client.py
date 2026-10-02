"""Cliente NMP real hacia server/. Desde la perspectiva de server/, este gateway
es simplemente "un cliente" más — server/ no sabe que existe un navegador detrás.

Transporte (docs/protocolo-NMP.md):
    TCP para REGISTER/EVENT/QUERY — mensajes terminados en '\n'.
    UDP para STATUS — un datagrama por mensaje; el servidor rechaza STATUS por TCP.
"""
import os
import socket

# Nombre de host, no IP fija: lo resuelve el sistema (DNS) al conectar.
NMP_HOST = os.environ.get("NMP_HOST", "localhost")
NMP_PORT = int(os.environ.get("NMP_PORT", "8080"))
NMP_TIMEOUT_S = float(os.environ.get("NMP_TIMEOUT", "5"))

# Límite del servidor: 511 bytes por mensaje (más el '\n').
MAX_LINE = 512


class NmpConnection:
    """Socket TCP con lectura por líneas para las respuestas terminadas en '\\n'."""

    def __init__(self, sock: socket.socket):
        self.sock = sock
        self.reader = sock.makefile("rb")

    def close(self) -> None:
        self.reader.close()
        self.sock.close()

    def __enter__(self) -> "NmpConnection":
        return self

    def __exit__(self, *exc) -> None:
        self.close()


def connect_nmp(host: str = NMP_HOST, port: int = NMP_PORT,
                timeout: float = NMP_TIMEOUT_S) -> NmpConnection:
    """Abre la conexión TCP. Lanza OSError si no se puede resolver o conectar."""
    return NmpConnection(socket.create_connection((host, port), timeout=timeout))


def send_nmp(conn: NmpConnection, message: str) -> str:
    """Envía un mensaje por TCP y devuelve la respuesta (una línea, sin '\\n').

    Lanza ConnectionError si el servidor cierra antes de completar la respuesta
    y OSError/TimeoutError ante fallos de red.
    """
    conn.sock.sendall(message.encode() + b"\n")

    line = conn.reader.readline(MAX_LINE)
    if not line.endswith(b"\n"):
        raise ConnectionError("el servidor cerró la conexión sin completar la respuesta")
    return line.decode().rstrip("\r\n")


def send_nmp_udp(message: str, host: str = NMP_HOST, port: int = NMP_PORT,
                 timeout: float = NMP_TIMEOUT_S) -> str:
    """Envía un STATUS como datagrama UDP y devuelve la respuesta (ACK o ERROR).

    Lanza TimeoutError si no llega respuesta: el datagrama o su ACK se perdieron.
    """
    family, _, _, _, addr = socket.getaddrinfo(host, port, type=socket.SOCK_DGRAM)[0]
    with socket.socket(family, socket.SOCK_DGRAM) as sock:
        sock.settimeout(timeout)
        sock.sendto(message.encode(), addr)
        data, _ = sock.recvfrom(MAX_LINE)
    return data.decode().rstrip("\r\n")
