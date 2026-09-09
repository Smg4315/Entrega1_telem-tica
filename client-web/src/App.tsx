import { useState } from "react";
import Login from "./components/Login";
import Dashboard from "./components/Dashboard";
import NodeDetail from "./components/NodeDetail";
import ProtocolLog from "./components/ProtocolLog";
import { mockNodes } from "./data/mockData";

type View = "dashboard" | "node";

const NAV_ITEMS = [
  {
    key: "dashboard",
    label: "Dashboard",
    icon: (
      <svg width="18" height="18" viewBox="0 0 18 18" fill="none">
        <rect x="1" y="1" width="7" height="7" rx="1.5" fill="currentColor" opacity="0.9" />
        <rect x="10" y="1" width="7" height="7" rx="1.5" fill="currentColor" opacity="0.5" />
        <rect x="1" y="10" width="7" height="7" rx="1.5" fill="currentColor" opacity="0.5" />
        <rect x="10" y="10" width="7" height="7" rx="1.5" fill="currentColor" opacity="0.9" />
      </svg>
    ),
  },
] as const;

function Sidebar({
  user,
  view,
  onNavigate,
  onLogout,
}: {
  user: string;
  view: View;
  onNavigate: (v: View) => void;
  onLogout: () => void;
}) {
  return (
    <aside
      className="flex flex-col shrink-0 w-16 sm:w-56"
      style={{ background: "linear-gradient(180deg, #2A2340 0%, #625A7E 100%)" }}
    >
      {/* Logo */}
      <div className="px-4 py-5 flex items-center gap-3 shrink-0" style={{ borderBottom: "1px solid #40366240" }}>
        <div className="w-8 h-8 rounded-lg shrink-0 flex items-center justify-center bg-white/20">
          <svg width="16" height="16" viewBox="0 0 20 20" fill="none">
            <circle cx="10" cy="10" r="3.5" fill="white" />
            <path d="M10 2v2.5M10 15.5V18M2 10h2.5M15.5 10H18" stroke="white" strokeWidth="1.6" strokeLinecap="round" />
            <path d="M4.5 4.5l1.4 1.4M14.1 14.1l1.4 1.4M4.5 15.5l1.4-1.4M14.1 5.9l1.4-1.4" stroke="white" strokeWidth="1.2" strokeLinecap="round" />
          </svg>
        </div>
        <div className="hidden sm:block">
          <div className="text-white font-bold text-sm tracking-wide leading-none">NetMonitor</div>
          <div className="text-white/40 text-xs mt-0.5">v1.0</div>
        </div>
      </div>

      {/* Nav */}
      <nav className="flex-1 px-2 py-4 flex flex-col gap-1">
        <div className="hidden sm:block px-2 mb-3">
          <div className="text-xs font-semibold uppercase tracking-widest" style={{ color: "#B3AFC060" }}>Navegación</div>
        </div>

        {NAV_ITEMS.map(item => {
          const active = view === item.key;
          return (
            <button
              key={item.key}
              onClick={() => onNavigate(item.key)}
              className="flex items-center gap-3 px-3 py-2.5 rounded-lg text-left w-full transition-all"
              style={{
                background: active ? "rgba(255,255,255,0.15)" : "transparent",
                color: active ? "#FFFFFF" : "#B3AFC0",
              }}
              onMouseEnter={e => { if (!active) e.currentTarget.style.background = "rgba(255,255,255,0.08)"; }}
              onMouseLeave={e => { if (!active) e.currentTarget.style.background = "transparent"; }}
            >
              <span style={{ opacity: active ? 1 : 0.7 }}>{item.icon}</span>
              <span className="hidden sm:block text-sm font-medium">{item.label}</span>
              {active && <span className="hidden sm:block ml-auto w-1.5 h-1.5 rounded-full bg-white" />}
            </button>
          );
        })}

        {/* Nodes list in sidebar */}
        <div className="hidden sm:block px-2 mt-4 mb-2">
          <div className="text-xs font-semibold uppercase tracking-widest" style={{ color: "#B3AFC060" }}>Nodos</div>
        </div>
        {mockNodes.map(node => {
          const dotColor = node.status === "disconnected" ? "#EF4444" : node.status === "active" ? "#22C55E" : "#6F828A";
          return (
            <div key={node.id} className="hidden sm:flex items-center gap-2.5 px-3 py-1.5">
              <span className="w-1.5 h-1.5 rounded-full shrink-0" style={{ background: dotColor }} />
              <span className="text-xs truncate" style={{ color: "#B3AFC090" }}>{node.id}</span>
            </div>
          );
        })}
      </nav>

      {/* User */}
      <div className="px-2 py-4 shrink-0" style={{ borderTop: "1px solid #40366240" }}>
        <div className="hidden sm:flex items-center gap-2.5 px-2 mb-2">
          <div className="w-7 h-7 rounded-full bg-white/20 flex items-center justify-center text-xs font-bold text-white shrink-0">
            {user.charAt(0).toUpperCase()}
          </div>
          <div className="min-w-0">
            <div className="text-xs font-semibold text-white truncate">{user}</div>
            <div className="text-xs" style={{ color: "#B3AFC060" }}>Administrador</div>
          </div>
        </div>
        <button
          onClick={onLogout}
          className="flex items-center gap-2.5 px-3 py-2 rounded-lg w-full transition-all"
          style={{ color: "#B3AFC0" }}
          onMouseEnter={e => { e.currentTarget.style.background = "rgba(255,255,255,0.08)"; e.currentTarget.style.color = "#FFFFFF"; }}
          onMouseLeave={e => { e.currentTarget.style.background = "transparent"; e.currentTarget.style.color = "#B3AFC0"; }}
        >
          <svg width="16" height="16" viewBox="0 0 16 16" fill="none">
            <path d="M6 14H3a1 1 0 01-1-1V3a1 1 0 011-1h3M11 11l3-3-3-3M14 8H6" stroke="currentColor" strokeWidth="1.4" strokeLinecap="round" strokeLinejoin="round" />
          </svg>
          <span className="hidden sm:block text-xs font-medium">Cerrar sesión</span>
        </button>
      </div>
    </aside>
  );
}

export default function App() {
  const [user, setUser] = useState<string | null>(null);
  const [view, setView] = useState<View>("dashboard");
  const [selectedNodeId, setSelectedNodeId] = useState<string | null>(null);
  const [showLog, setShowLog] = useState(true);

  if (!user) return <Login onLogin={setUser} />;

  const selectedNode = selectedNodeId ? mockNodes.find(n => n.id === selectedNodeId) : null;

  function handleSelectNode(id: string) {
    setSelectedNodeId(id);
    setView("node");
  }

  function handleBack() {
    setSelectedNodeId(null);
    setView("dashboard");
  }

  return (
    <div className="flex h-full" style={{ background: "#F7F7FA" }}>
      <Sidebar
        user={user}
        view={view}
        onNavigate={v => { setView(v); if (v === "dashboard") setSelectedNodeId(null); }}
        onLogout={() => setUser(null)}
      />

      {/* Main */}
      <div className="flex-1 flex flex-col min-w-0">
        {/* Top bar */}
        <header
          className="flex items-center justify-between px-6 py-3 shrink-0 bg-white"
          style={{ borderBottom: "1px solid #E5E7EB" }}
        >
          <div className="flex items-center gap-2">
            {view === "node" && selectedNode && (
              <button onClick={handleBack} className="mr-2 text-sm" style={{ color: "#9CA3AF" }}>←</button>
            )}
            <span className="text-sm font-semibold" style={{ color: "#1A1A2E" }}>
              {view === "dashboard" ? "Dashboard" : selectedNode?.label}
            </span>
            {view === "node" && selectedNode && (
              <span className="text-xs px-2 py-0.5 rounded" style={{ background: "#F3F4F6", color: "#6B7280", fontFamily: "JetBrains Mono, monospace" }}>
                {selectedNode.id}
              </span>
            )}
          </div>

          <div className="flex items-center gap-3">
            {/* Live pulse */}
            <div className="flex items-center gap-1.5">
              <span className="w-2 h-2 rounded-full animate-pulse" style={{ background: "#22C55E" }} />
              <span className="text-xs font-medium" style={{ color: "#22C55E" }}>LIVE</span>
            </div>

            <button
              onClick={() => setShowLog(v => !v)}
              className="text-xs px-3 py-1.5 rounded-lg border font-medium transition-all"
              style={{
                borderColor: showLog ? "#403662" : "#E5E7EB",
                color: showLog ? "#403662" : "#6B7280",
                background: showLog ? "#E8E7EC" : "#FFFFFF",
              }}
            >
              Log NMP
            </button>
          </div>
        </header>

        {/* Content */}
        <div className="flex flex-1 min-h-0" style={{ background: "#F7F7FA" }}>
          <div className="flex-1 min-w-0 overflow-auto">
            {view === "dashboard" && (
              <Dashboard nodes={mockNodes} onSelectNode={handleSelectNode} user={user} />
            )}
            {view === "node" && selectedNode && (
              <NodeDetail node={selectedNode} onBack={handleBack} />
            )}
          </div>

          {showLog && (
            <div className="w-60 shrink-0 hidden md:flex flex-col overflow-hidden">
              <ProtocolLog />
            </div>
          )}
        </div>
      </div>
    </div>
  );
}
