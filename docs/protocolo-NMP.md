# Protocolo NMP — spec de referencia

Fuente: `NetMonitor.docx`, secciones 4-9. 

## Tipos de mensaje

| Mensaje | Emisor → Receptor | Propósito |
|---|---|---|
| REGISTER | Nodo → Servidor | Registrar un nodo |
| STATUS | Nodo → Servidor | Enviar información periódica de estado |
| EVENT | Nodo → Servidor | Reportar un evento importante |
| ACK | Servidor → Nodo | Confirmar la recepción de un mensaje cuando corresponda |
| QUERY | Cliente → Servidor | Solicitar información actual o histórica |
| RESPONSE | Servidor → Cliente | Responder una consulta |
| ERROR | Servidor → Cliente/Nodo | Informar que una solicitud no pudo procesarse |

## Especificación del servicio

- **Registro:** el nodo solicita incorporarse con `REGISTER` y espera `ACK`. El servidor guarda su identificador y estado inicial.
- **Monitoreo:** el nodo envía `STATUS` periódicamente; el servidor actualiza su estado y conserva los datos necesarios para el historial.
- **Eventos:** el nodo usa `EVENT` para situaciones relevantes (alta temperatura, batería baja, fallas).
- **Consulta:** el cliente usa `QUERY` para pedir estado actual o histórico; recibe `RESPONSE`.
- **Errores:** el servidor usa `ERROR` cuando detecta mensajes o parámetros que no puede procesar.

## Sintaxis

Mensajes de texto, campos separados por `|`:
TIPO|ID|NODE_ID|DATOS


- **TIPO** — REGISTER, STATUS, EVENT, ACK, QUERY, RESPONSE o ERROR
- **ID** — identificador único del mensaje, correlaciona solicitud/respuesta
- **NODE_ID** — identificador del nodo involucrado
- **DATOS** — información específica de la operación

## Ejemplos

| Operación | Solicitud | Respuesta |
|---|---|---|
| Registro | `REGISTER\|001\|NODE01` | `ACK\|001\|REGISTER\|NODE01` |
| Estado | `STATUS\|002\|NODE01\|CPU=45\|MEM=62\|TEMP=38\|BAT=87` | `ACK\|002\|STATUS\|NODE01` |
| Evento | `EVENT\|003\|NODE01\|HIGH_TEMP\|TEMP=92` | `ACK\|003\|EVENT\|NODE01` |
| Consulta | `QUERY\|101\|NODE01\|CURRENT` | `RESPONSE\|101\|NODE01\|CURRENT\|CPU=45\|MEM=62\|TEMP=38\|BAT=87` |
| Error | — | `ERROR\|101\|UNKNOWN_NODE\|NODE99` |

## Reglas básicas de comunicación

- Un nodo debe registrarse antes de enviar estado o eventos.
- Cada nodo tiene un identificador único.
- Cada mensaje tiene un identificador para relacionar solicitud/respuesta.
- El servidor valida tipo, estructura y parámetros de los mensajes recibidos.
- Un nodo desconocido no puede enviar información al sistema.
- Los clientes deben autenticarse (LDAP) antes de realizar consultas.
- Los clientes no se comunican directamente con los nodos — solo con el servidor.
- El servidor mantiene el estado actual y el histórico necesario para las consultas.
- Las peticiones y respuestas relevantes se registran en logs.
- Ante desconexión o error, manejo controlado sin terminar todo el servicio.

## Máquina de estados — Nodo

| Estado | Descripción |
|---|---|
| Inicial | El nodo aún no está registrado |
| Registrando | Se envió `REGISTER`, se espera `ACK` |
| Activo | El nodo está registrado y puede enviar información |
| Enviando | Transmite métricas o eventos |
| Desconectado | Se detectó pérdida de comunicación; se intentará recuperar la conexión |

## Cliente administrativo

Si las credenciales LDAP son incorrectas, el cliente no tiene acceso a las funciones de consulta.

## Ejemplo de funcionamiento
NODO SERVIDOR CLIENTE
│──── REGISTER ───────────►│ │
│◄──── ACK ─────────────────│ │
│──── STATUS ───────────────►│ │
│◄──── ACK ─────────────────│ │
│──── EVENT ─────────────────►│ │
│◄──── ACK ─────────────────│ │
│ │◄──── QUERY ─────────────│
│ │──── RESPONSE ──────────►│


## Manejo de errores

| Situación | Respuesta / comportamiento |
|---|---|
| Mensaje desconocido | `ERROR\|INVALID_MESSAGE` |
| Formato incorrecto | `ERROR\|INVALID_FORMAT` |
| Nodo no registrado | `ERROR\|UNKNOWN_NODE` |
| Parámetro inválido | `ERROR\|INVALID_PARAMETER` |
| Usuario no autenticado | `ERROR\|UNAUTHORIZED` |
| Desconexión | Cierre controlado y cambio de estado |
| Fallo de DNS | Reintento de resolución sin cerrar el servidor |

## Transporte: TCP vs UDP

| Mensaje | Transporte | Justificación |
|---|---|---|
| REGISTER | TCP | El registro debe llegar correctamente y en orden |
| STATUS | UDP | Periódico; una pérdida ocasional se tolera porque llega una nueva actualización |
| EVENT | TCP | Información importante que no debería perderse |
| QUERY | TCP | El cliente necesita una respuesta confiable |
| RESPONSE | TCP | Debe recibirse completa y correctamente |
| ACK | Según el transporte del mensaje que confirma | Confirma cuando corresponde |
| ERROR | TCP | Debe entregarse correctamente la información del error |