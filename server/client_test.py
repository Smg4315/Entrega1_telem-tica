"""Cliente de prueba NMP.

Uso:
    python3 client_test.py                      # TCP, mensaje por defecto
    python3 client_test.py tcp "REGISTER|001|NODE01"
    python3 client_test.py udp "STATUS|002|NODE01|CPU=45"
"""
import socket
import sys

HOST = "127.0.0.1"
PORT = 8080

transport = sys.argv[1].lower() if len(sys.argv) > 1 else "tcp"
message = sys.argv[2] if len(sys.argv) > 2 else "REGISTER|1|NODE01|NetMonitor"

if transport == "tcp":
    # TCP: cada mensaje termina en '\n' (include/msg_io.h).
    payload = (message + "\n").encode()

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as client:
        client.connect((HOST, PORT))
        print("Conectado al servidor (TCP).")
        client.sendall(payload)

        # El servidor puede enviar la respuesta en varios segmentos:
        # se lee hasta el '\n', no un solo recv().
        raw = b""
        while not raw.endswith(b"\n"):
            chunk = client.recv(512)
            if not chunk:
                break
            raw += chunk

elif transport == "udp":
    # UDP: un datagrama por mensaje, sin '\n'.
    payload = message.encode()

    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as client:
        client.settimeout(3)
        print("Enviando datagrama al servidor (UDP).")
        client.sendto(payload, (HOST, PORT))
        raw, _ = client.recvfrom(512)

else:
    sys.exit("Transporte inválido: use tcp o udp")

print(f"Enviado  (raw): {payload!r}")
print(f"Recibido (raw): {raw!r}")

if transport == "tcp" and not raw.endswith(b"\n"):
    print("Error: el servidor cerró la conexión antes de completar la respuesta.")
else:
    print("Respuesta del servidor:")
    print(raw.decode().rstrip("\n"))
