import type { NodeStatus } from "../data/mockData";

const cfg: Record<NodeStatus, { label: string; color: string; bg: string; dot: string }> = {
  active:       { label: "Activo",       color: "#15803D", bg: "#F0FDF4", dot: "#22C55E" },
  sending:      { label: "Enviando",     color: "#6F828A", bg: "#C9EDFA", dot: "#6F828A" },
  registering:  { label: "Registrando",  color: "#B45309", bg: "#FFFBEB", dot: "#BE8156" },
  disconnected: { label: "Desconectado", color: "#B91C1C", bg: "#FEF2F2", dot: "#EF4444" },
};

export default function StatusBadge({ status }: { status: NodeStatus }) {
  const c = cfg[status];
  return (
    <span
      className="inline-flex items-center gap-1.5 px-2.5 py-1 rounded-full text-xs font-semibold"
      style={{ color: c.color, background: c.bg }}
    >
      <span className="w-1.5 h-1.5 rounded-full" style={{ background: c.dot }} />
      {c.label}
    </span>
  );
}
