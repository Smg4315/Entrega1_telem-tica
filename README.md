Claude


Readme · MD
# NetMonitor
 
Sistema distribuido de monitoreo y control de infraestructura — Telemática, Entrega 1.
 
Arquitectura de tres capas comunicadas por un protocolo propio (NMP) sobre sockets Berkeley, con un gateway que traduce hacia HTTP para el cliente web.
 
## Integrantes
 
- Viviana Arango Tabares
- Emmanuel Castaño Sepúlveda
- Jerónimo Contreras Sierra
- Simón Mazo Gómez
## Estado del proyecto
 
| Componente | Estado |
|---|---|
| `server/` | **Implementado** — concurrencia (fork por conexión TCP + hilo UDP, estado compartido), manejo de errores y validación de protocolo, transporte estricto por tipo de mensaje, control de registro de nodos |
| `gateway/` | **Implementado** — `connect_nmp`/`send_nmp` (TCP y UDP), endpoint QUERY conectado al servidor real, CORS habilitado. LDAP (`auth/ldap_bind.py`) sigue sin implementar |
| `client-web/` | **Funcional con datos reales** — dashboard conectado por polling al gateway, ya no usa mock data |
| `node-simulator/` | Sin implementar (las pruebas usan `server/client_test.py`) |
| `docs/protocolo-NMP.md` | Especificación actualizada para coincidir con el comportamiento real del servidor |
 
## Arquitectura
 
```
[Nodo] --NMP/TCP+UDP--> [server/] <--NMP/TCP+UDP-- [gateway/] <--HTTP--> [client-web/]
```
 
`server/` no distingue clientes: para él, `gateway/` es un cliente NMP más. El navegador no puede abrir sockets TCP/UDP crudos; el gateway resuelve esa limitación en nombre del cliente web.
 
## Componentes
 
| Carpeta | Responsabilidad | Puerto |
|---|---|---|
| `server/` | Servidor central NMP. Único componente con sockets Berkeley reales (TCP/UDP). | 8080 |
| `gateway/` | Traduce HTTP ↔ NMP. | 8011 |
| `client-web/` | Cliente administrativo (dashboard). Solo conoce HTTP. | 5173 |
| `node-simulator/` | Simulador de nodos NMP (pendiente). | — |
| `docs/` | Especificación del protocolo NMP. | — |
 
## Protocolo NMP
 
Formato de mensaje: `TIPO|ID|NODE_ID|DATOS`
 
Especificación completa, tipos de mensaje y códigos de error en [`docs/protocolo-NMP.md`](docs/protocolo-NMP.md).
 
## Requisitos (una sola vez)
 
1. Instalar WSL desde PowerShell como administrador:
```powershell
   wsl --install
```
2. Dentro de WSL, actualizar paquetes:
```bash
   sudo apt update
```
3. Dentro de WSL, instalar compilador y Python:
```bash
   sudo apt install -y build-essential python3-venv python3-pip netcat-openbsd
```
4. Tener instalado Node.js 18 o superior (en Windows o en WSL).
> El servidor requiere **Linux/WSL**: usa headers POSIX (`sys/socket.h`) que no existen en Windows nativo. El gateway y el cliente web pueden correr en Windows o en WSL.
>
> Linux distingue mayúsculas: la carpeta es `server`, no `Server`.
 
## Ejecución
 
Se necesitan **4 terminales abiertas al mismo tiempo**. Todos los comandos parten de la raíz del repositorio clonado.
 
### Terminal 1 — servidor (WSL)
 
1. Entrar a la carpeta del servidor:
```bash
   cd server
```
2. Compilar:
```bash
   make
```
3. Iniciar el servidor en el puerto 8080:
```bash
   ./bin/server 8080
```
4. Dejar esta terminal abierta (muestra los logs en vivo).
> Si `make` responde *"Nothing to be done for 'all'"*, el servidor ya está compilado: pasar directo al paso 3.
 
### Terminal 2 — gateway (WSL)
 
1. Entrar a la carpeta del gateway:
```bash
   cd gateway
```
2. Crear el entorno virtual (solo la primera vez):
```bash
   python3 -m venv .venv
```
3. Activar el entorno virtual:
```bash
   source .venv/bin/activate
```
4. Instalar dependencias (solo la primera vez):
```bash
   pip install -r requirements.txt
```
5. Iniciar el gateway:
```bash
   uvicorn app.main:app --host 0.0.0.0 --port 8011 --reload
```
 
**Alternativa en PowerShell** (pasos 2 y 3):
 
1. Crear el entorno virtual (solo la primera vez):
```powershell
   python -m venv .venv
```
2. Activar el entorno virtual:
```powershell
   .venv\Scripts\Activate.ps1
```
3. Solo si aparece *"la ejecución de scripts está deshabilitada"*, ejecutar una vez y repetir el paso anterior:
```powershell
   Set-ExecutionPolicy -Scope CurrentUser RemoteSigned
```
 
### Terminal 3 — cliente web (WSL o PowerShell)
 
1. Entrar a la carpeta del cliente:
```bash
   cd client-web
```
2. Instalar dependencias (solo la primera vez):
```bash
   npm install
```
3. Iniciar el dashboard:
```bash
   npm run dev
```
4. Abrir en el navegador: `http://localhost:5173`
### Terminal 4 — mensajes de prueba (WSL)
 
1. Entrar a la carpeta del servidor:
```bash
   cd server
```
2. Desde aquí se envían todos los mensajes de la demo (ver siguiente sección).
## Demo paso a paso
 
Todos los comandos se ejecutan en la **Terminal 4**, desde `server/`. Las comillas alrededor de cada mensaje son obligatorias (el carácter `|` es especial en la terminal).
 
### Parte 1 — Flujo normal
 
1. Registrar el nodo NODE01 por TCP:
```bash
   python3 client_test.py tcp "REGISTER|001|NODE01"
```
2. Resultado esperado: respuesta `ACK`.
3. Enviar métricas de NODE01 por UDP:
```bash
   python3 client_test.py udp "STATUS|002|NODE01|CPU=45|MEM=62|TEMP=38|BAT=87"
```
4. Resultado esperado: el servidor confirma la recepción.
5. Consultar el estado de NODE01 por TCP:
```bash
   python3 client_test.py tcp "QUERY|003|NODE01|CURRENT"
```
6. Resultado esperado: respuesta `RESPONSE` con las métricas enviadas.
7. Revisar el dashboard en `http://localhost:5173`: NODE01 aparece **Activo** con sus métricas (esperar unos segundos por el polling).
8. Registrar un segundo nodo:
```bash
   python3 client_test.py tcp "REGISTER|011|NODE02"
```
9. Enviar métricas del segundo nodo:
```bash
   python3 client_test.py udp "STATUS|012|NODE02|CPU=30|MEM=40|TEMP=35|BAT=95"
```
10. Resultado esperado: el dashboard muestra los dos nodos activos.
### Parte 2 — Manejo de errores
 
1. Consultar un nodo no registrado:
```bash
   python3 client_test.py tcp "QUERY|020|NODE99|CURRENT"
```
2. Resultado esperado: `ERROR` con código `UNKNOWN_NODE`.
3. Enviar STATUS por el transporte equivocado (TCP):
```bash
   python3 client_test.py tcp "STATUS|021|NODE01|CPU=50"
```
4. Resultado esperado: `ERROR` con código `INVALID_MESSAGE` (STATUS solo se acepta por UDP).
5. Enviar STATUS sin formato `KEY=VALUE`:
```bash
   python3 client_test.py udp "STATUS|022|NODE01|CPU"
```
6. Resultado esperado: `ERROR` con código `INVALID_PARAMETER`.
7. Enviar un mensaje con estructura incompleta:
```bash
   python3 client_test.py tcp "REGISTER"
```
8. Resultado esperado: `ERROR` con código `INVALID_FORMAT`.
### Parte 3 — Concurrencia
 
1. Abrir una **Terminal 5** (WSL) y dejar una conexión TCP abierta sin enviar nada:
```bash
   nc localhost 8080
```
2. Sin cerrar la Terminal 5, en la Terminal 4 registrar otro nodo:
```bash
   python3 client_test.py tcp "REGISTER|040|NODE03"
```
3. Resultado esperado: el `ACK` llega al instante, aunque la Terminal 5 mantiene otra conexión abierta (cada conexión TCP tiene su propio proceso).
4. Ver los procesos del servidor (un hijo por conexión abierta):
```bash
   ps -ef --forest | grep -v grep | grep "bin/server"
```
5. Con la Terminal 5 aún abierta, enviar un STATUS por UDP:
```bash
   python3 client_test.py udp "STATUS|041|NODE03|CPU=20|MEM=30|TEMP=33|BAT=99"
```
6. Resultado esperado: respuesta inmediata (el UDP lo atiende un hilo independiente).
7. Consultar el nodo desde otra conexión:
```bash
   python3 client_test.py tcp "QUERY|042|NODE03|CURRENT"
```
8. Resultado esperado: el nodo existe con sus métricas, lo que demuestra que todos los procesos comparten el mismo estado (memoria `mmap`).
9. Cerrar la Terminal 5 con `Ctrl + C` (o esperar 30 s: el servidor la cierra solo por `SO_RCVTIMEO`).
### Parte 4 — Timeout pasivo (nodo desconectado)
 
1. En la Terminal 1, detener el servidor: `Ctrl + C`
2. Reiniciarlo con un timeout corto de 10 segundos para la demo:
```bash
   NMP_NODE_TIMEOUT=10 ./bin/server 8080
```
3. En la Terminal 4, registrar el nodo:
```bash
   python3 client_test.py tcp "REGISTER|050|NODE01"
```
4. Enviar un único STATUS:
```bash
   python3 client_test.py udp "STATUS|051|NODE01|CPU=45|MEM=62|TEMP=38|BAT=87"
```
5. Consultar de inmediato:
```bash
   python3 client_test.py tcp "QUERY|052|NODE01|CURRENT"
```
6. Resultado esperado: respuesta normal (nodo activo).
7. Esperar más de 10 segundos sin enviar STATUS.
8. Consultar de nuevo:
```bash
   python3 client_test.py tcp "QUERY|053|NODE01|CURRENT"
```
9. Resultado esperado: la respuesta termina en `|DESCONECTADO` y el dashboard muestra el nodo como **Desconectado**.
10. Enviar un nuevo STATUS:
```bash
    python3 client_test.py udp "STATUS|054|NODE01|CPU=40|MEM=60|TEMP=37|BAT=86"
```
11. Resultado esperado: el siguiente QUERY y el dashboard vuelven a mostrar el nodo **Activo**, sin volver a registrarlo.
> Un QUERY no renueva la última señal del nodo: solo STATUS y EVENT lo hacen. Por defecto el timeout es de 60 s.
 
### Parte 5 — Caída del servidor
 
1. En la Terminal 1, detener el servidor: `Ctrl + C`
2. Intentar registrar un nodo desde la Terminal 4:
```bash
   python3 client_test.py tcp "REGISTER|060|NODE01"
```
3. Resultado esperado: `ConnectionRefusedError: [Errno 111] Connection refused` (no hay servidor escuchando en el puerto 8080).
4. Consultar al gateway desde una terminal libre **del mismo entorno donde corre el gateway**:
   - Si el gateway corre en WSL, usar una terminal WSL:
```bash
     curl -i http://127.0.0.1:8011/nodos/NODE01/historico
```
   - Si el gateway corre en PowerShell, usar PowerShell:
```powershell
     curl.exe -i http://127.0.0.1:8011/nodos/NODE01/historico
```
   - Alternativa sin terminal: abrir `http://127.0.0.1:8011/nodos/NODE01/historico` en el navegador.
5. Resultado esperado: HTTP `502` con `SERVER_UNREACHABLE` (el gateway captura el error de conexión y no se cae). El 502 también aparece en la Terminal 2, en el log de uvicorn.
   - Si en cambio sale `curl: (7) Failed to connect ... port 8011`, el `curl` no llegó al gateway: o el gateway no está corriendo, o está en el otro entorno (WSL / PowerShell). Para ver dónde escucha el puerto 8011, en WSL:
```bash
     ss -tlnp | grep 8011
```
   - En PowerShell:
```powershell
     netstat -ano | findstr 8011
```
6. Revisar el dashboard: los nodos pasan a estado **Error**, pero la página sigue funcionando.
7. Reiniciar el servidor en la Terminal 1:
```bash
   ./bin/server 8080
```
8. Volver a registrar el nodo (el registro vivía en memoria y se perdió):
```bash
   python3 client_test.py tcp "REGISTER|061|NODE01"
```
9. Enviar sus métricas:
```bash
   python3 client_test.py udp "STATUS|062|NODE01|CPU=45|MEM=62|TEMP=38|BAT=87"
```
10. Resultado esperado: el dashboard se recupera solo en el siguiente ciclo de polling, sin reiniciar el gateway ni el frontend.
> En PowerShell se usa `curl.exe` porque `curl` es un alias de otro comando. Desde WSL, `127.0.0.1` apunta a WSL y no a Windows: por eso el `curl` debe hacerse en el mismo entorno que el gateway.
 
#### Respuestas posibles del gateway
 
| Situación | Código HTTP | Contenido |
|---|---|---|
| Servidor apagado | `502` | `SERVER_UNREACHABLE` |
| Servidor encendido y nodo registrado | `200` | JSON con las métricas del nodo |
| Servidor encendido y nodo no registrado | `404` | `UNKNOWN_NODE` |
| Otro error del protocolo NMP | `400` | Código de error NMP |
 
## Decisiones de diseño
 
- Servidor 100 % C, sockets Berkeley puros — sin frameworks HTTP.
- Gateway en Python/FastAPI — capa de traducción NMP ↔ HTTP, sin lógica de negocio propia.
- `client-web` es el cliente administrativo real, no un prototipo.
- Concurrencia por `fork()` en TCP (un proceso por conexión) y un único hilo para UDP, con registro y estado de nodos en memoria compartida (`mmap`) protegida por un mutex entre procesos.
- Transporte estricto por tipo de mensaje: STATUS únicamente por UDP, el resto únicamente por TCP, según el diseño original del equipo.
- Robustez: timeout de 30 s por conexión TCP (`SO_RCVTIMEO`), `MSG_NOSIGNAL` contra `SIGPIPE`, rechazo de líneas vacías o demasiado largas, límite de 64 nodos con error `REGISTRY_FULL`.
- Detección de nodos inactivos por tiempo de espera (`NMP_NODE_TIMEOUT`, 60 s por defecto), no por reconexión activa.
### Desviaciones documentadas del diseño original
 
- **ERROR también se envía por UDP** cuando el mensaje inválido llegó por esa vía, ya que no existe una conexión TCP previa con ese emisor para responder por ese medio.
- **El estado "Desconectado" se detecta de forma pasiva** por ausencia de señal durante un tiempo configurable (`NMP_NODE_TIMEOUT`), sin intento de reconexión activa.
## Estructura del repositorio
 
```
netmonitor/
├── server/            # C — protocolo NMP sobre sockets Berkeley
├── gateway/           # Python — FastAPI, traducción NMP ↔ HTTP
├── client-web/        # React + TS — dashboard administrativo
├── node-simulator/    # Simulador de nodos (pendiente)
└── docs/
    └── protocolo-NMP.md
```
 
Claude finished the response
