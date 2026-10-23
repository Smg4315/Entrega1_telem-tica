# Etapa 1: compilación. gcc y debian slim comparten base (bookworm) para que
# el binario enlace contra la misma glibc en ejecución.
FROM gcc:14-bookworm AS build
WORKDIR /src
COPY . .
RUN make

# Etapa 2: solo el binario sobre una imagen mínima.
FROM debian:bookworm-slim
WORKDIR /app
COPY --from=build /src/bin/server ./server
# El servidor escucha TCP y UDP en el mismo puerto.
EXPOSE 8080/tcp 8080/udp
# stdbuf -oL: stdout sin búfer de bloque, para que los printf aparezcan en
# `docker compose logs` al momento (sin TTY, glibc los retiene).
CMD ["stdbuf", "-oL", "./server", "8080"]
