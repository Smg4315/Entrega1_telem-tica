interface Props {
  label: string;
  value: number | null; // null: el servidor no tiene esta métrica
  unit?: string;
  warn?: number;
  danger?: number;
  lowIsBad?: boolean; // batería: el valor bajo es el malo (value <= warn/danger)
}

export default function MetricGauge({ label, value, unit = "%", warn = 75, danger = 90, lowIsBad = false }: Props) {
  const v = value ?? 0;
  const isDanger = value !== null && (lowIsBad ? v <= danger : v >= danger);
  const isWarn = value !== null && (lowIsBad ? v <= warn : v >= warn);
  const good = lowIsBad ? "#22C55E" : "#403662";
  const color = value === null ? "#D1D5DB" : isDanger ? "#EF4444" : isWarn ? "#BE8156" : good;
  const track = isDanger ? "#FEE2E2" : isWarn ? "#F5ECE6" : lowIsBad && value !== null ? "#DCFCE7" : "#E8E7EC";
  const pct = Math.min(100, v);

  return (
    <div className="flex flex-col gap-1.5">
      <div className="flex justify-between items-baseline">
        <span className="text-xs font-medium uppercase tracking-wider" style={{ color: "#9CA3AF" }}>
          {label}
        </span>
        <span className="text-sm font-bold tabular-nums" style={{ color, fontFamily: "JetBrains Mono, monospace" }}>
          {value === null ? "--" : `${value}${unit}`}
        </span>
      </div>
      <div className="h-1.5 rounded-full overflow-hidden" style={{ background: track }}>
        <div
          className="h-full rounded-full transition-all duration-500"
          style={{ width: `${pct}%`, background: color }}
        />
      </div>
    </div>
  );
}
