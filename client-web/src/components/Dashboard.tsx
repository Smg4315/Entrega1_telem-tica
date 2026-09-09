import {
  AreaChart, Area, BarChart, Bar, XAxis, YAxis, Tooltip,
  ResponsiveContainer, CartesianGrid,
} from "recharts";
import type { MonitoredNode } from "../data/mockData";
import StatusBadge from "./StatusBadge";
import MetricGauge from "./MetricGauge";

interface Props {
  nodes: MonitoredNode[];
  onSelectNode: (id: string) => void;
  user: string;
}

const STATUS_COLOR: Record<string, string> = {
  active: "#22C55E",
  sending: "#6F828A",
  registering: "#BE8156",
  disconnected: "#EF4444",
};

function NodeCard({ node, onSelect }: { node: MonitoredNode; onSelect: () => void }) {
  const isDown = node.status === "disconnected";

  return (
    <button
      onClick={onSelect}
      className="text-left w-full rounded-xl border p-5 transition-all bg-white group"
      style={{ borderColor: "#E5E7EB", cursor: "pointer" }}
      onMouseEnter={e => { e.currentTarget.style.borderColor = "#403662"; e.currentTarget.style.boxShadow = "0 4px 20px #40366215"; }}
      onMouseLeave={e => { e.currentTarget.style.borderColor = "#E5E7EB"; e.currentTarget.style.boxShadow = "none"; }}
    >
      {/* Header */}
      <div className="flex items-start justify-between mb-4 gap-2">
        <div className="min-w-0">
          <div className="font-semibold text-sm truncate" style={{ color: "#1A1A2E" }}>
            {node.label}
          </div>
          <div className="text-xs mt-0.5" style={{ color: "#9CA3AF", fontFamily: "JetBrains Mono, monospace" }}>
            {node.id} · {node.location}
          </div>
        </div>
        <StatusBadge status={node.status} />
      </div>

      {/* Mini sparkline (beige bg) */}
      {!isDown && (
        <div className="rounded-lg p-3 mb-4" style={{ background: "#F5F1EB" }}>
          <ResponsiveContainer width="100%" height={48}>
            <AreaChart data={node.history.slice(-8)} margin={{ top: 2, right: 2, left: -36, bottom: 0 }}>
              <defs>
                <linearGradient id={`grad-${node.id}`} x1="0" y1="0" x2="0" y2="1">
                  <stop offset="0%" stopColor="#403662" stopOpacity={0.3} />
                  <stop offset="100%" stopColor="#403662" stopOpacity={0} />
                </linearGradient>
              </defs>
              <Area type="monotone" dataKey="cpu" stroke="#403662" strokeWidth={1.5} fill={`url(#grad-${node.id})`} dot={false} />
              <XAxis dataKey="timestamp" hide />
              <YAxis hide domain={[0, 100]} />
              <Tooltip
                contentStyle={{ background: "#fff", border: "1px solid #E5E7EB", borderRadius: 8, fontSize: 11 }}
                formatter={(v) => [`${v}%`, "CPU"]}
                labelFormatter={() => ""}
              />
            </AreaChart>
          </ResponsiveContainer>
          <div className="text-xs mt-1" style={{ color: "#89829E", fontFamily: "JetBrains Mono, monospace" }}>CPU — últimas lecturas</div>
        </div>
      )}

      {isDown ? (
        <div className="py-4 text-center text-xs rounded-lg" style={{ background: "#FEF2F2", color: "#EF4444" }}>
          Nodo sin respuesta — desconectado
        </div>
      ) : (
        <div className="flex flex-col gap-2.5">
          <MetricGauge label="MEM" value={node.current.mem} />
          <MetricGauge label="TEMP" value={node.current.temp} unit="°C" warn={70} danger={85} />
          <MetricGauge label="BAT" value={node.current.bat} warn={30} danger={15} />
        </div>
      )}

      <div className="flex items-center justify-between mt-4 pt-3" style={{ borderTop: "1px solid #F3F4F6" }}>
        <span className="text-xs" style={{ color: "#9CA3AF" }}>Señal: {node.lastSeen}</span>
        <span className="text-xs font-medium opacity-0 group-hover:opacity-100 transition-opacity" style={{ color: "#403662" }}>
          Ver detalle →
        </span>
      </div>
    </button>
  );
}

export default function Dashboard({ nodes, onSelectNode, user }: Props) {
  const active = nodes.filter(n => n.status === "active" || n.status === "sending").length;
  const disconnected = nodes.filter(n => n.status === "disconnected").length;
  const events = nodes.reduce((acc, n) => acc + n.events.length, 0);

  // Aggregate CPU history across all nodes (avg)
  const cpuTrend = nodes[0]?.history.map((h, i) => ({
    timestamp: h.timestamp,
    avg: Math.round(nodes.filter(n => n.status !== "disconnected").reduce((s, n) => s + n.history[i]?.cpu, 0) / active),
  })) ?? [];

  const statusDist = [
    { label: "Activo", count: nodes.filter(n => n.status === "active").length, color: "#22C55E" },
    { label: "Enviando", count: nodes.filter(n => n.status === "sending").length, color: "#6F828A" },
    { label: "Desconectado", count: nodes.filter(n => n.status === "disconnected").length, color: "#EF4444" },
  ];

  return (
    <div className="p-6 flex flex-col gap-6 overflow-auto h-full">
      {/* Greeting */}
      <div>
        <h1 className="text-xl font-bold" style={{ color: "#1A1A2E" }}>
          Bienvenido, <span style={{ color: "#403662" }}>{user}</span>
        </h1>
        <p className="text-sm mt-0.5" style={{ color: "#6B7280" }}>
          Panel de monitoreo — {new Date().toLocaleDateString("es-CO", { weekday: "long", year: "numeric", month: "long", day: "numeric" })}
        </p>
      </div>

      {/* KPI row */}
      <div className="grid grid-cols-2 lg:grid-cols-4 gap-4">
        {[
          { label: "Nodos totales", value: nodes.length, sub: "registrados", color: "#403662", bg: "#E8E7EC" },
          { label: "En línea", value: active, sub: "activos / enviando", color: "#15803D", bg: "#F0FDF4" },
          { label: "Desconectados", value: disconnected, sub: "sin respuesta", color: "#B91C1C", bg: "#FEF2F2" },
          { label: "Eventos activos", value: events, sub: "requieren atención", color: "#B45309", bg: "#FFFBEB" },
        ].map(s => (
          <div key={s.label} className="rounded-xl border p-5 bg-white" style={{ borderColor: "#E5E7EB" }}>
            <div className="flex items-start justify-between">
              <div>
                <div className="text-3xl font-bold tabular-nums" style={{ color: s.color, fontFamily: "JetBrains Mono, monospace" }}>
                  {s.value}
                </div>
                <div className="font-semibold text-sm mt-1" style={{ color: "#1A1A2E" }}>{s.label}</div>
                <div className="text-xs mt-0.5" style={{ color: "#9CA3AF" }}>{s.sub}</div>
              </div>
              <div className="w-9 h-9 rounded-lg flex items-center justify-center text-base" style={{ background: s.bg, color: s.color }}>
                ●
              </div>
            </div>
          </div>
        ))}
      </div>

      {/* Charts row */}
      <div className="grid grid-cols-1 lg:grid-cols-3 gap-4">
        {/* CPU trend — beige bg */}
        <div className="lg:col-span-2 rounded-xl border p-5 bg-white" style={{ borderColor: "#E5E7EB" }}>
          <div className="flex items-center justify-between mb-4">
            <div>
              <div className="font-semibold text-sm" style={{ color: "#1A1A2E" }}>Tendencia CPU promedio</div>
              <div className="text-xs mt-0.5" style={{ color: "#9CA3AF" }}>Últimas 10 lecturas — nodos activos</div>
            </div>
          </div>
          <div className="rounded-xl p-4" style={{ background: "#F5F1EB" }}>
            <ResponsiveContainer width="100%" height={140}>
              <AreaChart data={cpuTrend} margin={{ top: 4, right: 4, left: -20, bottom: 0 }}>
                <defs>
                  <linearGradient id="cpuGrad" x1="0" y1="0" x2="0" y2="1">
                    <stop offset="0%" stopColor="#403662" stopOpacity={0.25} />
                    <stop offset="100%" stopColor="#403662" stopOpacity={0} />
                  </linearGradient>
                </defs>
                <CartesianGrid strokeDasharray="3 6" stroke="#E6E3DD" />
                <XAxis dataKey="timestamp" tick={{ fill: "#89829E", fontSize: 10 }} tickLine={false} axisLine={false} />
                <YAxis tick={{ fill: "#89829E", fontSize: 10 }} tickLine={false} axisLine={false} domain={[0, 100]} />
                <Tooltip
                  contentStyle={{ background: "#fff", border: "1px solid #E5E7EB", borderRadius: 8, fontSize: 11 }}
                  formatter={(v) => [`${v}%`, "CPU avg"]}
                />
                <Area type="monotone" dataKey="avg" stroke="#403662" strokeWidth={2} fill="url(#cpuGrad)" dot={false} activeDot={{ r: 4, fill: "#403662" }} />
              </AreaChart>
            </ResponsiveContainer>
          </div>
        </div>

        {/* Status distribution */}
        <div className="rounded-xl border p-5 bg-white" style={{ borderColor: "#E5E7EB" }}>
          <div className="font-semibold text-sm mb-1" style={{ color: "#1A1A2E" }}>Distribución de estados</div>
          <div className="text-xs mb-4" style={{ color: "#9CA3AF" }}>Nodos por estado actual</div>
          <div className="rounded-xl p-4" style={{ background: "#F5F1EB" }}>
            <ResponsiveContainer width="100%" height={140}>
              <BarChart data={statusDist} margin={{ top: 4, right: 4, left: -20, bottom: 0 }}>
                <CartesianGrid strokeDasharray="3 6" stroke="#E6E3DD" vertical={false} />
                <XAxis dataKey="label" tick={{ fill: "#89829E", fontSize: 9 }} tickLine={false} axisLine={false} />
                <YAxis tick={{ fill: "#89829E", fontSize: 10 }} tickLine={false} axisLine={false} allowDecimals={false} />
                <Tooltip
                  contentStyle={{ background: "#fff", border: "1px solid #E5E7EB", borderRadius: 8, fontSize: 11 }}
                  formatter={(v) => [v, "nodos"]}
                />
                <Bar dataKey="count" radius={[4, 4, 0, 0]}>
                  {statusDist.map((entry, i) => (
                    <rect key={i} fill={entry.color} />
                  ))}
                </Bar>
              </BarChart>
            </ResponsiveContainer>
          </div>
          {/* Legend */}
          <div className="flex flex-col gap-2 mt-4">
            {statusDist.map(s => (
              <div key={s.label} className="flex items-center justify-between">
                <div className="flex items-center gap-2">
                  <span className="w-2.5 h-2.5 rounded-full" style={{ background: s.color }} />
                  <span className="text-xs" style={{ color: "#6B7280" }}>{s.label}</span>
                </div>
                <span className="text-xs font-bold tabular-nums" style={{ color: "#1A1A2E", fontFamily: "JetBrains Mono, monospace" }}>{s.count}</span>
              </div>
            ))}
          </div>
        </div>
      </div>

      {/* Node cards */}
      <div>
        <h2 className="text-sm font-semibold mb-3" style={{ color: "#6B7280" }}>
          NODOS REGISTRADOS
        </h2>
        <div className="grid grid-cols-1 sm:grid-cols-2 xl:grid-cols-3 gap-4">
          {nodes.map(node => (
            <NodeCard key={node.id} node={node} onSelect={() => onSelectNode(node.id)} />
          ))}
        </div>
      </div>
    </div>
  );
}
