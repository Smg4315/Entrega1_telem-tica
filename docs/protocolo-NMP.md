# Protocolo NMP — spec de referencia

Fuente: `NetMonitor.docx`, secciones 4-9, y la propuesta original del equipo en la wiki.

Este documento describe el comportamiento real del servidor (`server/`). Donde el texto original y el código no coincidían, se documenta lo que hace el código. Lo que está documentado en el diseño pero no implementado se marca como **no implementado en Fase 3**.

## Tipos de mensaje

| Mensaje | Emisor → Receptor | Propósito |
|---|---|---|
| REGISTER | Nodo → Servidor | Registrar un nodo |
| STATUS | Nodo → Servidor | Enviar información periódica de estado |
| EVENT | Nodo → Servidor | Reportar un evento importante |
| ACK | Servidor → Nodo | Confirmar la recepción de un mensaje cuando corresponda |
| QUERY | Cliente → Servidor | Solicitar el estado actual de un nodo (histórico: no implementado en Fase 3) |
| RESPONSE | Servidor → Cliente | Responder una consulta |
| ERROR | Servidor → Cliente/Nodo | Informar que una solicitud no pudo procesarse |

## Especificación del servicio

- **Registro:** el nodo solicita incorporarse con `REGISTER` y espera `ACK`. El servidor guarda su identificador e inicia su marca de última actividad.
- **Monitoreo:** el nodo envía `STATUS` periódicamente; el servidor actualiza el estado actual del nodo (último valor de cada métrica).
- **Eventos:** el nodo usa `EVENT` para situaciones relevantes (alta temperatura, batería baja, fallas). Los pares `CLAVE=VALOR` del evento se fusionan en el estado actual; el nombre del evento no se almacena.
- **Consulta:** el cliente usa `QUERY` para pedir el estado actual; recibe `RESPONSE`.
- **Errores:** el servidor usa `ERROR` cuando detecta mensajes o parámetros que no puede procesar.

## Sintaxis

Mensajes de texto, campos separados por `|`. El orden de los campos depende del tipo:

| Tipos | Formato |
|---|---|
| REGISTER, STATUS, EVENT, QUERY, RESPONSE | `TIPO\|ID\|NODE_ID\|DATOS` |
| ACK | `ACK\|ID\|TIPO_CONFIRMADO\|NODE_ID` |
| ERROR | `ERROR\|ID\|CODIGO\|NODE_ID` |

- **TIPO** — REGISTER, STATUS, EVENT, ACK, QUERY, RESPONSE o ERROR.
- **ID** — identificador del mensaje; solo dígitos. La respuesta repite el ID de la solicitud tal como llegó (conserva ceros a la izquierda: `001`).
- **NODE_ID** — identificador del nodo involucrado (máximo 31 caracteres).
- **DATOS** — información específica de la operación (máximo 255 caracteres). Puede contener `|`: todo lo que sigue al tercer separador es DATOS.
- **TIPO_CONFIRMADO** — tipo del mensaje que se confirma: `REGISTER`, `STATUS` o `EVENT`.
- **CODIGO** — código de error (ver "Manejo de errores").

> **Nota sobre el orden de campos.** En ACK y ERROR el tercer campo no es el NODE_ID: el nodo va al final. La versión anterior de esta sección declaraba un único formato `TIPO|ID|NODE_ID|DATOS` para todos los tipos, lo cual contradecía los ejemplos. Esa inconsistencia ya estaba en la propuesta original del equipo en la wiki, no solo en este archivo. Se conserva el formato de los ejemplos, que es el que implementa el servidor.

DATOS por tipo de solicitud:

| Tipo | DATOS |
|---|---|
| REGISTER | Opcional; se ignora |
| STATUS | Obligatorio: al menos un par `CLAVE=VALOR`; varios pares se separan con `\|` |
| EVENT | Obligatorio: al menos un par `CLAVE=VALOR`; puede ir precedido del nombre del evento (`HIGH_TEMP\|TEMP=92`) |
| QUERY | `CURRENT` (único valor aceptado en Fase 3) |

## Ejemplos

| Operación | Transporte | Solicitud | Respuesta |
|---|---|---|---|
| Registro | TCP | `REGISTER\|001\|NODE01` | `ACK\|001\|REGISTER\|NODE01` |
| Estado | UDP | `STATUS\|002\|NODE01\|CPU=45\|MEM=62\|TEMP=38\|BAT=87` | `ACK\|002\|STATUS\|NODE01` |
| Evento | TCP | `EVENT\|003\|NODE01\|HIGH_TEMP\|TEMP=92` | `ACK\|003\|EVENT\|NODE01` |
| Consulta | TCP | `QUERY\|101\|NODE01\|CURRENT` | `RESPONSE\|101\|NODE01\|CURRENT\|CPU=45\|MEM=62\|TEMP=92\|BAT=87` |
| Consulta, nodo sin métricas | TCP | `QUERY\|102\|NODE01\|CURRENT` | `RESPONSE\|102\|NODE01\|CURRENT` |
| Consulta, nodo inactivo | TCP | `QUERY\|103\|NODE01\|CURRENT` | `RESPONSE\|103\|NODE01\|CURRENT\|CPU=45\|MEM=62\|TEMP=92\|BAT=87\|DESCONECTADO` |
| Error, nodo no registrado | TCP | `QUERY\|101\|NODE99\|CURRENT` | `ERROR\|101\|UNKNOWN_NODE\|NODE99` |
| Error, mensaje ilegible | TCP | `hola` | `ERROR\|0\|INVALID_FORMAT\|` |

## Reglas básicas de comunicación

- Un nodo debe registrarse antes de enviar estado o eventos.
- Cada nodo tiene un identificador único. Repetir `REGISTER` para un nodo ya registrado es válido y devuelve `ACK`.
- Cada mensaje tiene un identificador para relacionar solicitud/respuesta.
- El servidor valida tipo, estructura y parámetros de los mensajes recibidos.
- Un nodo desconocido no puede enviar información al sistema, ni puede ser consultado.
- Los clientes no se comunican directamente con los nodos — solo con el servidor.
- El servidor mantiene el estado actual de cada nodo.
- Las peticiones y respuestas se registran en `logs/server.log`.
- Ante desconexión o error, manejo controlado sin terminar todo el servicio.
- Los clientes deben autenticarse (LDAP) antes de realizar consultas. **Documentado, no implementado en Fase 3:** el servidor atiende `QUERY` sin autenticación.
- El servidor mantiene el histórico necesario para las consultas. **Documentado, no implementado en Fase 3:** solo se guarda el último valor de cada métrica.

## Máquina de estados — Nodo

| Estado | Descripción |
|---|---|
| Inicial | El nodo aún no está registrado |
| Registrando | Se envió `REGISTER`, se espera `ACK` |
| Activo | El nodo está registrado y puede enviar información |
| Enviando | Transmite métricas o eventos |
| Desconectado | Se detectó pérdida de comunicación |

En Fase 3 el servidor detecta el estado Desconectado de forma pasiva, por tiempo sin actividad (ver "Fase 3 — Vida del nodo"). El diseño original indicaba que "se intentará recuperar la conexión"; ese reintento activo queda fuera del alcance de Fase 3.

## Cliente administrativo

Si las credenciales LDAP son incorrectas, el cliente no tiene acceso a las funciones de consulta. **Documentado, no implementado en Fase 3.**

## Ejemplo de funcionamiento
```
NODO SERVIDOR CLIENTE
│──── REGISTER ───────────►│ │
│◄──── ACK ─────────────────│ │
│──── STATUS ───────────────►│ │
│◄──── ACK ─────────────────│ │
│──── EVENT ─────────────────►│ │
│◄──── ACK ─────────────────│ │
│ │◄──── QUERY ─────────────│
│ │──── RESPONSE ──────────►│
```
## Manejo de errores

Todo error se responde con un mensaje de cuatro campos:

```
ERROR|<id>|<codigo>|<node_id>
```

- `<id>` y `<node_id>` se copian de la solicitud.
- Si la solicitud no se pudo interpretar, no hay ID ni nodo que copiar: la respuesta es `ERROR|0|INVALID_FORMAT|` (ID `0`, NODE_ID vacío).

| Código | Cuándo se devuelve | Ejemplo de solicitud | Respuesta |
|---|---|---|---|
| `INVALID_FORMAT` | El mensaje no se puede interpretar: línea vacía, línea demasiado larga, faltan campos, ID no numérico, NODE_ID vacío o de más de 31 caracteres | `STATUS\|abc\|NODE01\|CPU=1` | `ERROR\|0\|INVALID_FORMAT\|` |
| `INVALID_MESSAGE` | La estructura es correcta, pero el tipo no es aceptable: tipo desconocido, tipo que solo emite el servidor (ACK, RESPONSE, ERROR) o tipo enviado por el transporte equivocado | `FOO\|1\|NODE01\|x` | `ERROR\|1\|INVALID_MESSAGE\|NODE01` |
| `UNKNOWN_NODE` | STATUS, EVENT o QUERY sobre un nodo que no ha hecho `REGISTER` | `QUERY\|101\|NODE99\|CURRENT` | `ERROR\|101\|UNKNOWN_NODE\|NODE99` |
| `INVALID_PARAMETER` | STATUS o EVENT sin ningún par `CLAVE=VALOR` en DATOS; QUERY con un valor distinto de `CURRENT` | `EVENT\|4\|NODE01\|HIGH_TEMP` | `ERROR\|4\|INVALID_PARAMETER\|NODE01` |
| `UNAUTHORIZED` | Usuario no autenticado. **Documentado, no implementado en Fase 3:** el servidor nunca lo emite | — | — |

Orden de validación del servidor: (1) estructura y tipo conocido, (2) transporte, (3) nodo registrado, (4) tipo válido como solicitud y parámetros. Un mensaje con varios problemas recibe solo el primer error de esa lista.

Consecuencia de ese orden: si un cliente envía por TCP un ACK, RESPONSE o ERROR, el servidor toma su tercer campo como NODE_ID. Si ese valor no es un nodo registrado, la respuesta es `UNKNOWN_NODE`; solo si lo es, la respuesta es `INVALID_MESSAGE`. Por UDP la respuesta es siempre `INVALID_MESSAGE`.

Otras situaciones:

| Situación | Comportamiento |
|---|---|
| Desconexión del cliente | El proceso que atiende esa conexión la cierra y termina; el resto del servidor sigue funcionando |
| Consulta histórica (`QUERY ... HISTORY`) | **Documentado, no implementado en Fase 3:** devuelve `ERROR\|<id>\|INVALID_PARAMETER\|<node_id>` |
| Fallo de DNS | El diseño prevé reintento de resolución sin cerrar el servidor. **Documentado, no implementado en Fase 3:** el servidor no hace resolución de nombres |

## Transporte: TCP vs UDP

| Mensaje | Transporte | Justificación |
|---|---|---|
| REGISTER | TCP | El registro debe llegar correctamente y en orden |
| STATUS | UDP | Periódico; una pérdida ocasional se tolera porque llega una nueva actualización |
| EVENT | TCP | Información importante que no debería perderse |
| QUERY | TCP | El cliente necesita una respuesta confiable |
| RESPONSE | TCP | Debe recibirse completa y correctamente |
| ACK | Según el transporte del mensaje que confirma | Confirma cuando corresponde |
| ERROR | TCP (con una excepción, ver "Fase 3 — ERROR por UDP") | Debe entregarse correctamente la información del error |

TCP y UDP escuchan en el mismo número de puerto.

## Fase 3 — Concurrencia, límites y casos de error

### Concurrencia

- **TCP:** un proceso hijo (`fork`) por conexión. Un cliente lento o bloqueado no detiene a los demás.
- **UDP:** un hilo dedicado atiende todos los datagramas.
- El registro de nodos y el estado se guardan en memoria compartida protegida por un mutex, de modo que todos los procesos y el hilo UDP ven los mismos datos.

### Delimitación de mensajes (framing)

- **TCP:** cada mensaje termina en `\n`. Una conexión puede llevar varios mensajes seguidos; cada uno recibe su respuesta, también terminada en `\n`.
- Se acepta `\r\n`: el `\r` final se descarta.
- **UDP:** un datagrama es un mensaje; no requiere `\n` (si lo trae, se descarta).

### Límites

| Límite | Valor | Qué pasa si se excede |
|---|---|---|
| Longitud de mensaje TCP | 511 bytes (sin contar `\n`) | `ERROR\|0\|INVALID_FORMAT\|`; el resto de la línea se descarta hasta el `\n` y la conexión sigue abierta |
| Línea vacía (TCP) | — | `ERROR\|0\|INVALID_FORMAT\|`; la conexión sigue abierta (no se confunde con un cierre) |
| Longitud de datagrama UDP | 511 bytes | Se procesan solo los primeros 511 bytes |
| ID | Solo dígitos, máximo 15, valor máximo 4294967295 | `INVALID_FORMAT` |
| NODE_ID | 31 caracteres | `INVALID_FORMAT` |
| DATOS | 255 caracteres | `INVALID_FORMAT` |
| Nodos registrados | 64 | Limitación conocida: el nodo 65 recibe `ACK` pero no queda registrado (el servidor lo informa en su salida de error); sus mensajes posteriores reciben `UNKNOWN_NODE` |
| Métricas por nodo | 16; clave y valor de hasta 31 caracteres cada uno | El par que excede el límite se ignora sin error |

### Transporte estricto

El servidor aplica la tabla de transporte del diseño original sin excepciones de entrada:

| Recibido | Respuesta |
|---|---|
| `STATUS` por TCP | `ERROR\|<id>\|INVALID_MESSAGE\|<node_id>` |
| Cualquier tipo distinto de `STATUS` por UDP | `ERROR\|<id>\|INVALID_MESSAGE\|<node_id>` |

Esta comprobación va antes que la de nodo registrado: un `STATUS` por TCP de un nodo desconocido recibe `INVALID_MESSAGE`, no `UNKNOWN_NODE`.

### ERROR por UDP

El diseño original indica que ERROR viaja solo por TCP. Hay un caso en que eso no es posible: cuando un `STATUS` (que llega por UDP) falla una validación del servidor, por ejemplo nodo no registrado o DATOS sin `CLAVE=VALOR`. El remitente de ese datagrama no tiene ninguna conexión TCP abierta con el servidor, así que no existe un canal TCP por el cual devolverle el error. El servidor responde entonces con un datagrama `ERROR` a la dirección de origen.

Esto es una consecuencia inevitable de separar los transportes, no una desviación por conveniencia: el diseño original no previó que un tipo de mensaje exclusivo de UDP pudiera fallar la validación en el servidor. Como cualquier datagrama, ese `ERROR` puede perderse.

### Tiempo de espera en TCP

- Cada conexión TCP tiene un tiempo máximo de inactividad de 30 segundos (`TCP_RECV_TIMEOUT_S`).
- Si el cliente no envía nada en ese plazo, el servidor cierra la conexión sin enviar mensaje. El cliente debe abrir una conexión nueva; el registro del nodo se conserva.

### Vida del nodo (estado Desconectado)

- El servidor guarda por nodo la hora del último mensaje aceptado (`last_seen`). Se actualiza con cada `REGISTER`, `STATUS` o `EVENT` que recibe `ACK`. Un `QUERY` sobre el nodo no la actualiza.
- Si el nodo lleva en silencio más que la ventana configurada, la respuesta a `QUERY` agrega `|DESCONECTADO` al final del estado.
- La ventana se configura con la variable de entorno `NMP_NODE_TIMEOUT` (segundos) al arrancar el servidor; por defecto 60.
- El nodo sigue registrado. En cuanto vuelve a enviar un mensaje válido, `DESCONECTADO` deja de aparecer.
- La detección es pasiva: se evalúa solo al responder un `QUERY`. El servidor no intenta reconectar ni avisa a nadie. El reintento activo que sugiere la máquina de estados original queda fuera del alcance de Fase 3.

### Desconexiones y fallos de comunicación

| Situación | Comportamiento |
|---|---|
| El cliente cierra la conexión entre mensajes | El proceso hijo termina normalmente |
| El cliente cierra con un mensaje a medias (sin `\n`) | El fragmento se descarta; el proceso hijo termina |
| El cliente se desconecta mientras el servidor responde | El envío usa `MSG_NOSIGNAL`: falla con `EPIPE` en lugar de matar al proceso con `SIGPIPE`. El error se informa en la salida de error del servidor |
| Lectura o escritura parcial | El servidor reintenta hasta completar el mensaje |
| Llamada interrumpida por una señal (`EINTR`) | Se reintenta |
| Reinicio de conexión (`ECONNRESET`) | Se cierra esa conexión; el servidor sigue atendiendo a los demás |

### Pérdida y duplicación de mensajes

- **Pérdida (UDP):** un `STATUS` o su `ACK` pueden perderse. El servidor no retransmite ni exige retransmisión; el siguiente `STATUS` periódico repone el dato. Si el nodo no recibe `ACK`, no puede saber cuál de los dos se perdió, y reenviar es seguro.
- **Duplicación:** el servidor no recuerda los ID ya vistos. Un mensaje repetido se procesa de nuevo y recibe la misma respuesta. No causa daño: `REGISTER` repetido no crea un segundo registro, y `STATUS`/`EVENT` repetidos reescriben los mismos valores.
- **TCP:** la pérdida y la duplicación las resuelve el propio transporte.
