"""Parsing/serialización de NMP: TIPO|ID|NODE_ID|DATOS (docx, sección 4).
Mismo formato que el servidor en C — se reimplementa aquí, no se comparte código con server/.
"""
from dataclasses import dataclass

NMP_TYPES = {"REGISTER", "STATUS", "EVENT", "ACK", "QUERY", "RESPONSE", "ERROR"}


@dataclass
class NmpMessage:
    type: str
    id: str
    node_id: str
    data: str = ""  # resto de campos, ya unidos con "|"


# ACK y ERROR llevan el nodo al final: TIPO|ID|DATOS|NODE_ID
# (ej. ACK|001|REGISTER|NODE01, ERROR|101|UNKNOWN_NODE|NODE99).
# El resto usa TIPO|ID|NODE_ID|DATOS, y DATOS puede contener más '|'.
NODE_LAST_TYPES = {"ACK", "ERROR"}


def parse_nmp(raw: str) -> NmpMessage:
    parts = raw.split("|")
    msg_type = parts[0]
    if msg_type not in NMP_TYPES:
        raise ValueError(f"INVALID_MESSAGE: tipo desconocido '{msg_type}'")
    if len(parts) < 3:
        raise ValueError(f"INVALID_FORMAT: faltan campos en '{raw}'")

    msg_id = parts[1]
    if msg_type in NODE_LAST_TYPES:
        # ERROR|0|INVALID_FORMAT| trae NODE_ID vacío.
        data = parts[2]
        node_id = parts[3] if len(parts) > 3 else ""
    else:
        node_id = parts[2]
        data = "|".join(parts[3:])
    return NmpMessage(type=msg_type, id=msg_id, node_id=node_id, data=data)


def serialize_nmp(msg: NmpMessage) -> str:
    if msg.type in NODE_LAST_TYPES:
        return "|".join([msg.type, msg.id, msg.data, msg.node_id])
    parts = [msg.type, msg.id, msg.node_id]
    if msg.data:
        parts.append(msg.data)
    return "|".join(parts)
