import socket

HOST = "127.0.0.1"
PORT = 8080

message = "INVALID|5|NODE01|TEST\n"

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as client:
    client.connect((HOST, PORT))

    print("Conectado al servidor.")

    client.sendall(message.encode())

    response = client.recv(512).decode()

    print("Respuesta del servidor:")
    print(response)