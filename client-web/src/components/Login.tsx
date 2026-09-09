import { useState } from "react";

interface Props {
  onLogin: (user: string) => void;
}

export default function Login({ onLogin }: Props) {
  const [user, setUser] = useState("");
  const [pass, setPass] = useState("");
  const [error, setError] = useState("");
  const [loading, setLoading] = useState(false);

  function handleSubmit(e: React.FormEvent) {
    e.preventDefault();
    if (!user || !pass) { setError("Ingresa usuario y contraseña."); return; }
    setLoading(true);
    setError("");
    setTimeout(() => {
      if (pass.length >= 4) {
        onLogin(user);
      } else {
        setError("Credenciales incorrectas. Verifica tus datos.");
        setLoading(false);
      }
    }, 900);
  }

  return (
    <div className="min-h-screen flex">
      {/* Purple left panel */}
      <div
        className="hidden lg:flex lg:w-1/2 flex-col justify-between p-12"
        style={{ background: "linear-gradient(160deg, #2A2340 0%, #403662 100%)" }}
      >
        {/* Logo */}
        <div className="flex items-center gap-3">
          <div className="w-9 h-9 rounded-lg flex items-center justify-center bg-white/20">
            <svg width="20" height="20" viewBox="0 0 20 20" fill="none">
              <circle cx="10" cy="10" r="3.5" fill="white" />
              <path d="M10 2v2.5M10 15.5V18M2 10h2.5M15.5 10H18" stroke="white" strokeWidth="1.6" strokeLinecap="round" />
              <path d="M4.5 4.5l1.8 1.8M13.7 13.7l1.8 1.8M4.5 15.5l1.8-1.8M13.7 6.3l1.8-1.8" stroke="white" strokeWidth="1.2" strokeLinecap="round" />
            </svg>
          </div>
          <span className="text-white font-bold text-lg tracking-wide">NetMonitor</span>
        </div>

        {/* Center copy */}
        <div>
          <h1 className="text-4xl font-bold text-white leading-tight mb-4">
            Monitoreo de<br />infraestructura<br />distribuida
          </h1>
          <p className="text-white/60 text-sm leading-relaxed max-w-xs">
            Visualiza en tiempo real el estado de tus nodos. CPU, memoria, temperatura y batería centralizados en un solo panel.
          </p>

          {/* Decorative stat pills */}
          <div className="flex gap-3 mt-8">
            {[
              { label: "Nodos activos", val: "4/5" },
              { label: "Protocolo", val: "NMP/1.0" },
              { label: "Transporte", val: "TCP+UDP" },
            ].map(s => (
              <div key={s.label} className="px-3 py-2 rounded-lg bg-white/10">
                <div className="text-white font-semibold text-sm">{s.val}</div>
                <div className="text-white/50 text-xs mt-0.5">{s.label}</div>
              </div>
            ))}
          </div>
        </div>

        <p className="text-white/30 text-xs">Telemática — Fase 1 · 2026</p>
      </div>

      {/* White right panel */}
      <div className="flex-1 flex items-center justify-center p-8" style={{ background: "#FFFFFF" }}>
        <div className="w-full max-w-sm">
          {/* Mobile logo */}
          <div className="flex items-center gap-2.5 mb-8 lg:hidden">
            <div className="w-8 h-8 rounded-lg flex items-center justify-center" style={{ background: "#403662" }}>
              <svg width="16" height="16" viewBox="0 0 20 20" fill="none">
                <circle cx="10" cy="10" r="3.5" fill="white" />
                <path d="M10 2v2.5M10 15.5V18M2 10h2.5M15.5 10H18" stroke="white" strokeWidth="1.6" strokeLinecap="round" />
              </svg>
            </div>
            <span className="font-bold text-base" style={{ color: "#342C50" }}>NetMonitor</span>
          </div>

          <h2 className="text-2xl font-bold mb-1" style={{ color: "#1A1A2E" }}>Iniciar sesión</h2>
          <p className="text-sm mb-8" style={{ color: "#6B7280" }}>
            Acceso para clientes administrativos via LDAP
          </p>

          <form onSubmit={handleSubmit} className="flex flex-col gap-4">
            <div>
              <label className="block text-xs font-semibold mb-1.5 uppercase tracking-wider" style={{ color: "#6B7280" }}>
                Usuario
              </label>
              <input
                type="text"
                value={user}
                onChange={e => setUser(e.target.value)}
                placeholder="ej. admin"
                autoComplete="username"
                className="w-full px-4 py-3 text-sm rounded-lg border outline-none transition-all"
                style={{ borderColor: "#E5E7EB", color: "#1A1A2E", background: "#FAFAFA" }}
                onFocus={e => { e.target.style.borderColor = "#403662"; e.target.style.background = "#FFFFFF"; e.target.style.boxShadow = "0 0 0 3px #E8E7EC"; }}
                onBlur={e => { e.target.style.borderColor = "#E5E7EB"; e.target.style.background = "#FAFAFA"; e.target.style.boxShadow = "none"; }}
              />
            </div>

            <div>
              <label className="block text-xs font-semibold mb-1.5 uppercase tracking-wider" style={{ color: "#6B7280" }}>
                Contraseña
              </label>
              <input
                type="password"
                value={pass}
                onChange={e => setPass(e.target.value)}
                placeholder="••••••••"
                autoComplete="current-password"
                className="w-full px-4 py-3 text-sm rounded-lg border outline-none transition-all"
                style={{ borderColor: "#E5E7EB", color: "#1A1A2E", background: "#FAFAFA" }}
                onFocus={e => { e.target.style.borderColor = "#403662"; e.target.style.background = "#FFFFFF"; e.target.style.boxShadow = "0 0 0 3px #E8E7EC"; }}
                onBlur={e => { e.target.style.borderColor = "#E5E7EB"; e.target.style.background = "#FAFAFA"; e.target.style.boxShadow = "none"; }}
              />
            </div>

            {error && (
              <div className="px-4 py-3 rounded-lg text-sm" style={{ background: "#FEF2F2", color: "#DC2626", border: "1px solid #FECACA" }}>
                {error}
              </div>
            )}

            <button
              type="submit"
              disabled={loading}
              className="mt-2 py-3 text-sm font-semibold rounded-lg transition-all"
              style={{
                background: loading ? "#89829E" : "#403662",
                color: "#FFFFFF",
                cursor: loading ? "not-allowed" : "pointer",
                boxShadow: loading ? "none" : "0 4px 14px #40366240",
              }}
            >
              {loading ? "Verificando credenciales..." : "Acceder al sistema"}
            </button>
          </form>

          <p className="text-center text-xs mt-6" style={{ color: "#D1D5DB" }}>
            LDAP · NMP/1.0 · NetMonitor v1.0
          </p>
        </div>
      </div>
    </div>
  );
}
