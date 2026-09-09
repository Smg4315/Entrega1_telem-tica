"""POST /login — recibe usuario/contraseña del front (client-web/Login.tsx) y delega en LDAP."""
from fastapi import APIRouter, HTTPException
from pydantic import BaseModel
from ..auth.ldap_bind import ldap_bind

router = APIRouter()


class LoginRequest(BaseModel):
    user: str
    password: str


@router.post("/login")
async def login(body: LoginRequest):
    try:
        await ldap_bind(body.user, body.password)
        return {"ok": True, "user": body.user}
    except Exception:
        raise HTTPException(status_code=401, detail="UNAUTHORIZED")
