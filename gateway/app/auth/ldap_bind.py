"""Única pieza que habla LDAP en nombre del cliente web — el navegador no puede
hacer bind LDAP directo.

TODO: usar ldap3 contra el servidor LDAP real del docx (sección 2), ej.:
    from ldap3 import Server, Connection
    conn = Connection(Server(LDAP_HOST), user=f"uid={user},...", password=password)
    if not conn.bind(): raise PermissionError
"""


async def ldap_bind(user: str, password: str) -> None:
    raise NotImplementedError("ldap_bind: pendiente de conectar con el servidor LDAP real")
