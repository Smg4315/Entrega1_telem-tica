import {
  AreaChart, Area, XAxis, YAxis, Tooltip, ResponsiveContainer, CartesianGrid,
} from "recharts";
import type { MonitoredNode, NodeEvent } from "../data/mockData";
import StatusBadge from "./StatusBadge";

const EVENT_STYLE: Record<NodeEvent["type"], { color: string; bg: string; label: string }> = {
  HIGH_TEMP: { color: "#B45309", bg: "#FFFBEB", label: "Alta temperatura" },
  LOW_BAT:   { color: "#B91C1C", bg: "#FEF2F2", label: "Batería baja" },
  DISCONNECT:{ color: "#B91C1C", bg: "#FEF2F2", label: "Desconexión" },
  RECONNECT: { color: "#15803D", bg: "#F0FDF4", label: "Reconexión" },
  FAULT:     { color: "#7C2D12", bg: "#FFF7ED", label: "Fallo" },
};

interface ChartCardProps {
  title: string;
  sub: string;
  dataKey: string;
  color: string;
  data: object[];
  unit?: string;
  gradId: string;
}

function ChartCard({ title, sub, dataKey, color, data, unit = "%", gradId }: ChartCardProps) {
  return (
    <div className="rounded-xl border p-5 bg-white" style={{ borderColor: "#E5E7EB" }}>
      <div className="font-semibold text-sm mb-0.5" style={{ color: "#1A1A2E" }}>{title}</div>
      <div className="text-xs mb-4" style={{ color: "#9CA3AF" }}>{sub}</div>
      <div className="rounded-xl p-4" style={{ background: "#F5F1EB" }}>
        <ResponsiveContainer width="100%" height={110}>
          <AreaChart data={data} margin={{ top: 2, right: 2, left: -24, bottom: 0 }}>
            <defs>
              <linearGradient id={gradId} x1="0" y1="0" x2="0" y2="1">
                <stop offset="0%" stopColor={color} stopOpacity={0.25} />
                <stop offset="100%" stopColor={color} stopOpacity={0} />
              </linearGradient>
            </defs>
            <CartesianGrid strokeDasharray="3 6" stroke="#E6E3DD" />
            <XAxis dataKey="timestamp" tick={{ fill: "#89829E", fontSize: 9 }} tickLine={false} axisLine={false} />
            <YAxis tick={{ fill: "#89829E", fontSize: 9 }} tickLine={false} axisLine={false} domain={[0, 100]} />
            <Tooltip
              contentStyle={{ background: "#fff", border: "1px solid #E5E7EB", borderRadius: 8, fontSize: 11 }}
              formatter={(v) => [`${v}${unit}`, title]}
              labelStyle={{ color: "#6B7280" }}
            />
            <Area type="monotone" dataKey={dataKey} stroke={color} strokeWidth={2} fill={`url(#${gradId})`} dot={false} activeDot={{ r: 4, fill: color }} />
          </AreaChart>
        </ResponsiveContainer>
      </div>
    </div>
  );
}

interface Props {
  node: MonitoredNode;
  onBack: () => void;
}

export default function NodeDetail({ node, onBack }: Props) {
  const last5 = node.history.slice(-5);
  const isDown = node.status === "disconnected";

  return (
    <div className="p-6 flex flex-col gap-6 overflow-auto h-full">
      {/* Back */}
      <div>
        <button
          onClick={onBack}
          className="text-xs font-medium mb-4 inline-flex items-center gap-1.5 transition-colors"
          style={{ color: "#9CA3AF" }}
          onMouseEnter={e => (e.currentTarget.style.color = "#403662")}
          onMouseLeave={e => (e.currentTarget.style.color = "#9CA3AF")}
        >
          ← Volver al dashboard
        </button>

        <div className="flex flex-col sm:flex-row sm:items-center sm:justify-between gap-3">
          <div>
            <h1 className="text-xl font-bold" style={{ color: "#1A1A2E" }}>{node.label}</h1>
            <p className="text-xs mt-1" style={{ color: "#9CA3AF", fontFamily: "JetBrains Mono, monospace" }}>
              {node.id} · {node.location} · Última señal: {node.lastSeen}
            </p>
          </div>
          <StatusBadge status={node.status} />
        </div>
      </div>

      {/* Current metrics */}
      <div className="grid grid-cols-2 lg:grid-cols-4 gap-4">
        {[
          { label: "CPU", value: node.current.cpu, unit: "%", warn: 75, danger: 90 },
          { label: "Memoria", value: node.current.mem, unit: "%", warn: 75, danger: 90 },
          { label: "Temperatura", value: node.current.temp, unit: "°C", warn: 70, danger: 85 },
          { label: "Batería", value: node.current.bat, unit: "%", warn: 30, danger: 15 },
        ].map(m => {
          const color = isDown ? "#D1D5DB" : m.value >= m.danger ? "#EF4444" : m.value >= m.warn ? "#BE8156" : "#403662";
          const bg = isDown ? "#F9FAFB" : m.value >= m.danger ? "#FEF2F2" : m.value >= m.warn ? "#FFFBEB" : "#E8E7EC";
          return (
            <div key={m.label} className="rounded-xl border p-5 bg-white" style={{ borderColor: "#E5E7EB" }}>
              <div className="text-3xl font-bold tabular-nums" style={{ color, fontFamily: "JetBrains Mono, monospace" }}>
                {isDown ? "--" : `${m.value}${m.unit}`}
              </div>
              <div className="text-xs font-semibold mt-2" style={{ color: "#1A1A2E" }}>{m.label}</div>
              <div className="mt-2 h-1.5 rounded-full overflow-hidden" style={{ background: bg }}>
                {!isDown && (
                  <div className="h-full rounded-full" style={{ width: `${Math.min(100, m.value)}%`, background: color }} />
                )}
              </div>
            </div>
          );
        })}
      </div>

      {/* Charts */}
      <div>
        <h2 className="text-sm font-semibold mb-3" style={{ color: "#6B7280" }}>HISTÓRICO — ÚLTIMAS 10 LECTURAS</h2>
        <div className="grid grid-cols-1 sm:grid-cols-2 gap-4">
          <ChartCard title="CPU" sub="Uso del procesador" dataKey="cpu" color="#403662" data={node.history} gradId="d-cpu" />
          <ChartCard title="Memoria" sub="Uso de RAM" dataKey="mem" color="#6F828A" data={node.history} gradId="d-mem" />
          <ChartCard title="Temperatura" sub="Temperatura del sistema" dataKey="temp" color="#BE8156" data={node.history} unit="°C" gradId="d-temp" />
          <ChartCard title="Batería" sub="Nivel de carga" dataKey="bat" color="#22C55E" data={node.history} gradId="d-bat" />
        </div>
      </div>

      {/* Last 5 readings table */}
      <div>
        <h2 className="text-sm font-semibold mb-3" style={{ color: "#6B7280" }}>ÚLTIMAS 5 LECTURAS DE ESTADO</h2>
        <div className="rounded-xl border overflow-hidden bg-white" style={{ borderColor: "#E5E7EB" }}>
          <table className="w-full text-sm">
            <thead>
              <tr style={{ background: "#F9FAFB", borderBottom: "1px solid #E5E7EB" }}>
                {["Hora", "CPU", "Memoria", "Temperatura", "Batería"].map(h => (
                  <th key={h} className="text-left px-5 py-3 text-xs font-semibold uppercase tracking-wider" style={{ color: "#6B7280" }}>{h}</th>
                ))}
              </tr>
            </thead>
            <tbody>
              {last5.map((row, i) => (
                <tr key={i} style={{ borderBottom: i < last5.length - 1 ? "1px solid #F3F4F6" : "none" }}>
                  <td className="px-5 py-3 text-xs" style={{ color: "#9CA3AF", fontFamily: "JetBrains Mono, monospace" }}>{row.timestamp}</td>
                  <td className="px-5 py-3 text-xs font-semibold tabular-nums" style={{ color: row.cpu >= 90 ? "#EF4444" : row.cpu >= 75 ? "#BE8156" : "#403662", fontFamily: "JetBrains Mono, monospace" }}>{row.cpu}%</td>
                  <td className="px-5 py-3 text-xs font-semibold tabular-nums" style={{ color: row.mem >= 90 ? "#EF4444" : row.mem >= 75 ? "#BE8156" : "#6F828A", fontFamily: "JetBrains Mono, monospace" }}>{row.mem}%</td>
                  <td className="px-5 py-3 text-xs font-semibold tabular-nums" style={{ color: row.temp >= 85 ? "#EF4444" : row.temp >= 70 ? "#BE8156" : "#1A1A2E", fontFamily: "JetBrains Mono, monospace" }}>{row.temp}°C</td>
                  <td className="px-5 py-3 text-xs font-semibold tabular-nums" style={{ color: row.bat <= 15 ? "#EF4444" : row.bat <= 30 ? "#BE8156" : "#22C55E", fontFamily: "JetBrains Mono, monospace" }}>{row.bat}%</td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      </div>

      {/* Events */}
      <div>
        <h2 className="text-sm font-semibold mb-3" style={{ color: "#6B7280" }}>EVENTOS REGISTRADOS</h2>
        {node.events.length === 0 ? (
          <div className="rounded-xl border px-5 py-8 text-center text-sm bg-white" style={{ borderColor: "#E5E7EB", color: "#D1D5DB" }}>
            Sin eventos para este nodo
          </div>
        ) : (
          <div className="flex flex-col gap-3">
            {node.events.map(ev => {
              const s = EVENT_STYLE[ev.type];
              return (
                <div key={ev.id} className="rounded-xl border px-5 py-4 flex items-start gap-4 bg-white" style={{ borderColor: "#E5E7EB" }}>
                  <div className="w-8 h-8 rounded-lg flex items-center justify-center text-sm shrink-0" style={{ background: s.bg, color: s.color }}>⚠</div>
                  <div className="flex-1 min-w-0">
                    <div className="flex items-center gap-2 mb-1">
                      <span className="text-xs font-bold uppercase tracking-wider" style={{ color: s.color }}>{s.label}</span>
                      <span className="text-xs px-1.5 py-0.5 rounded" style={{ background: "#F3F4F6", color: "#9CA3AF", fontFamily: "JetBrains Mono, monospace" }}>{ev.id}</span>
                    </div>
                    <p className="text-sm" style={{ color: "#374151" }}>{ev.message}</p>
                  </div>
                  <span className="text-xs shrink-0 mt-0.5" style={{ color: "#9CA3AF", fontFamily: "JetBrains Mono, monospace" }}>{ev.timestamp}</span>
                </div>
              );
            })}
          </div>
        )}
      </div>
    </div>
  );
}
