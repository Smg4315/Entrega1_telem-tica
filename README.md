
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
   sudo apt install -y build-essential python3-venv python3-pip
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
 
### Terminal 2 — gateway (PowerShell)
 
1. Abrir una ventana de PowerShell y entrar a la carpeta del gateway:
```powershell
   cd gateway
```
2. Crear el entorno virtual (solo la primera vez):
```powershell
   python -m venv .venv
```
3. Activar el entorno virtual:
```powershell
   .venv\Scripts\Activate.ps1
```
4. Solo si aparece *"la ejecución de scripts está deshabilitada"*, ejecutar una vez y repetir el paso 3:
```powershell
   Set-ExecutionPolicy -Scope CurrentUser RemoteSigned
```
5. Instalar dependencias (solo la primera vez):
```powershell
   pip install -r requirements.txt
```
6. Iniciar el gateway:
```powershell
   uvicorn app.main:app --host 0.0.0.0 --port 8011 --reload
```
7. Resultado esperado: la terminal muestra `Uvicorn running on http://0.0.0.0:8011`.

 
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
 
### Parte 1 — Creación de dos nodos
 
1. Registrar el nodo NODE01 por TCP:
```bash
   python3 client_test.py tcp "REGISTER|001|NODE01"
```
2. Resultado esperado: respuesta `ACK`.
3. Enviar métricas de NODE01 por UDP:
```bash
   python3 client_test.py udp "STATUS|002|NODE01|CPU=45|MEM=62|TEMP=38|BAT=87"
```
4. Consultar el estado de NODE01 por TCP:
```bash
   python3 client_test.py tcp "QUERY|003|NODE01|CURRENT"
```
5. Resultado esperado: respuesta `RESPONSE` con las métricas enviadas (CPU, MEM, TEMP y BAT).

6. Registrar el nodo NODE02 por TCP:
```bash
   python3 client_test.py tcp "REGISTER|011|NODE02"
```
7. Resultado esperado: respuesta `ACK`.
   
8. Enviar métricas de NODE02 por UDP:
```bash
   python3 client_test.py udp "STATUS|012|NODE02|CPU=30|MEM=40|TEMP=35|BAT=95"
```
9. Revisar el dashboard en `http://localhost:5173`: NODE01 y NODE02 aparecen **Activos** con sus métricas (esperar unos segundos por el polling).
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
### Parte 3 — Timeout pasivo (nodo desconectado)
 
Se usa NODE01, registrado en la Parte 1, con el servidor encendido y la configuración por defecto (60 s).
 
1. Enviar un STATUS de NODE01 para reiniciar su última señal:
```bash
   python3 client_test.py udp "STATUS|050|NODE01|CPU=45|MEM=62|TEMP=38|BAT=87"
```
2. Consultar de inmediato:
```bash
   python3 client_test.py tcp "QUERY|051|NODE01|CURRENT"
```
3. Resultado esperado: respuesta normal (nodo activo).
4. Esperar más de 60 segundos sin enviar STATUS de NODE01.
5. Consultar de nuevo:
```bash
   python3 client_test.py tcp "QUERY|052|NODE01|CURRENT"
```
6. Resultado esperado: la respuesta termina en `|DESCONECTADO` y el dashboard muestra NODE01 como **Desconectado**.
7. Enviar un nuevo STATUS:
```bash
   python3 client_test.py udp "STATUS|053|NODE01|CPU=40|MEM=60|TEMP=37|BAT=86"
```
8. Consultar otra vez:
```bash
   python3 client_test.py tcp "QUERY|054|NODE01|CURRENT"
```
9. Resultado esperado: NODE01 vuelve a estar **Activo** en la respuesta y en el dashboard, sin volver a registrarlo.
> El servidor no se reinicia en ningún momento. Un QUERY no renueva la última señal del nodo: solo STATUS y EVENT lo hacen. El tiempo de 60 s es el valor por defecto de `NMP_NODE_TIMEOUT`.
 
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
 
