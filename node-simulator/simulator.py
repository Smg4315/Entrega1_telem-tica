"""Simulador de nodos NMP: cada nodo se registra por TCP, envía STATUS periódico
por UDP y reporta EVENT por TCP cuando una métrica cruza un umbral.

Los EVENT no se pierden si el nodo se queda sin red: quedan en una cola y se
reenvían, en orden y con su ID original, hasta recibir ACK (entrega "al menos
una vez"; el servidor acepta duplicados sin daño).

Uso:
    python simulator.py                 # NODE01 y NODE02 contra localhost:8080
    NODES=NODE01,NODE03 INTERVAL=2 python simulator.py

Variables de entorno: NMP_HOST, NMP_PORT, NODES (separados por coma), INTERVAL (s).
"""
import collections
import itertools
import os
import random
import socket
import threading
import time

NMP_HOST = os.environ.get("NMP_HOST", "localhost")
NMP_PORT = int(os.environ.get("NMP_PORT", "8080"))
NODES = [n.strip() for n in os.environ.get("NODES", "NODE01,NODE02").split(",") if n.strip()]
INTERVAL_S = float(os.environ.get("INTERVAL", "3"))
TIMEOUT_S = 3

HIGH_TEMP = 80
LOW_BAT = 15

# Tope de EVENT pendientes por nodo: en un corte muy largo se descartan los más viejos.
MAX_PENDING_EVENTS = 100

# ID numérico por mensaje, compartido por todos los hilos (el servidor solo acepta dígitos).
_ids = itertools.count(1)
_ids_lock = threading.Lock()


def next_id() -> int:
    with _ids_lock:
        return next(_ids)


def log(node_id: str, sent: str, reply: str) -> None:
    print(f"[{node_id}] {sent}  ->  {reply}", flush=True)


def send_tcp(message: str) -> str:
    """REGISTER/EVENT: mensaje terminado en '\\n', respuesta de una línea."""
    with socket.create_connection((NMP_HOST, NMP_PORT), timeout=TIMEOUT_S) as sock:
        sock.sendall(message.encode() + b"\n")
        with sock.makefile("rb") as reader:
            return reader.readline(512).decode().rstrip("\r\n")


def send_udp(message: str) -> str:
    """STATUS: un datagrama, sin '\\n'."""
    family, _, _, _, addr = socket.getaddrinfo(NMP_HOST, NMP_PORT, type=socket.SOCK_DGRAM)[0]
    with socket.socket(family, socket.SOCK_DGRAM) as sock:
        sock.settimeout(TIMEOUT_S)
        sock.sendto(message.encode(), addr)
        data, _ = sock.recvfrom(512)
    return data.decode().rstrip("\r\n")


def drift(value: float, step: float, low: float, high: float) -> float:
    """Paseo aleatorio acotado: las métricas cambian poco entre lecturas."""
    return min(high, max(low, value + random.uniform(-step, step)))


class SimulatedNode:
    def __init__(self, node_id: str):
        self.node_id = node_id
        self.cpu = random.uniform(20, 60)
        self.mem = random.uniform(30, 70)
        self.temp = random.uniform(35, 55)
        self.bat = random.uniform(60, 100)
        self.hot = False
        self.low_bat = False
        self.pending_events = collections.deque(maxlen=MAX_PENDING_EVENTS)

    def register(self) -> None:
        """Reintenta hasta que el servidor acepte el REGISTER (puede no haber arrancado)."""
        while True:
            msg = f"REGISTER|{next_id()}|{self.node_id}|SIMULATOR"
            try:
                reply = send_tcp(msg)
                log(self.node_id, msg, reply)
                if reply.startswith("ACK"):
                    return
            except OSError as exc:
                log(self.node_id, msg, f"sin conexión ({exc})")
            time.sleep(INTERVAL_S)

    def step(self) -> None:
        self.cpu = drift(self.cpu, 12, 1, 100)
        self.mem = drift(self.mem, 5, 5, 98)
        # La temperatura sigue a la CPU, con algo de ruido.
        self.temp = drift(0.7 * self.temp + 0.3 * (30 + self.cpu * 0.6), 4, 25, 99)
        # La batería se descarga y, al agotarse, se recarga completa.
        self.bat = self.bat - random.uniform(0.5, 2) if self.bat > 5 else 100

    def send_status(self) -> None:
        msg = (f"STATUS|{next_id()}|{self.node_id}|CPU={self.cpu:.0f}|MEM={self.mem:.0f}"
               f"|TEMP={self.temp:.0f}|BAT={self.bat:.0f}")
        try:
            reply = send_udp(msg)
        except OSError as exc:  # incluye timeout: datagrama o ACK perdidos
            log(self.node_id, msg, f"sin respuesta ({exc})")
            return
        log(self.node_id, msg, reply)
        # El servidor se reinició y perdió el registro: volver a registrarse.
        if reply.startswith("ERROR") and "UNKNOWN_NODE" in reply:
            self.register()

    def queue_event(self, name: str, pair: str) -> None:
        self.pending_events.append(f"EVENT|{next_id()}|{self.node_id}|{name}|{pair}")
        self.flush_events()

    def flush_events(self) -> None:
        """Envía los EVENT pendientes en orden; se detiene en el primer fallo de red
        para no reordenarlos, y el resto espera al siguiente ciclo."""
        while self.pending_events:
            msg = self.pending_events[0]
            try:
                reply = send_tcp(msg)
            except OSError as exc:
                log(self.node_id, msg, f"sin conexión, en cola: {len(self.pending_events)} ({exc})")
                return
            log(self.node_id, msg, reply)
            if reply.startswith("ERROR") and "UNKNOWN_NODE" in reply:
                self.register()  # se reintenta el mismo EVENT ya registrado
                continue
            # ACK, o un ERROR que no se arregla reenviando: sale de la cola.
            self.pending_events.popleft()

    def check_events(self) -> None:
        """Un EVENT al cruzar cada umbral, no uno por lectura."""
        hot = self.temp >= HIGH_TEMP
        if hot and not self.hot:
            self.queue_event("HIGH_TEMP", f"TEMP={self.temp:.0f}")
        self.hot = hot

        low = self.bat <= LOW_BAT
        if low and not self.low_bat:
            self.queue_event("LOW_BAT", f"BAT={self.bat:.0f}")
        self.low_bat = low

    def run(self) -> None:
        self.register()
        while True:
            self.step()
            # Pendientes antes del STATUS: el servidor fusiona los pares del EVENT
            # en el estado, y así el STATUS siguiente deja los valores actuales.
            self.flush_events()
            self.send_status()
            self.check_events()
            time.sleep(INTERVAL_S)


def main() -> None:
    print(f"Simulando {', '.join(NODES)} contra {NMP_HOST}:{NMP_PORT} cada {INTERVAL_S}s", flush=True)
    threads = [threading.Thread(target=SimulatedNode(n).run, daemon=True) for n in NODES]
    for t in threads:
        t.start()
    for t in threads:
        t.join()


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        pass
