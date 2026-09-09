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


def parse_nmp(raw: str) -> NmpMessage:
    parts = raw.split("|")
    msg_type, msg_id, node_id, *rest = parts + [""] * (4 - len(parts))
    if msg_type not in NMP_TYPES:
        raise ValueError(f"INVALID_MESSAGE: tipo desconocido '{msg_type}'")
    return NmpMessage(type=msg_type, id=msg_id, node_id=node_id, data="|".join(rest))


def serialize_nmp(msg: NmpMessage) -> str:
    parts = [msg.type, msg.id, msg.node_id]
    if msg.data:
        parts.append(msg.data)
    return "|".join(parts)
