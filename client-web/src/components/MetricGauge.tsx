interface Props {
  label: string;
  value: number;
  unit?: string;
  warn?: number;
  danger?: number;
}

export default function MetricGauge({ label, value, unit = "%", warn = 75, danger = 90 }: Props) {
  const color = value >= danger ? "#EF4444" : value >= warn ? "#BE8156" : "#403662";
  const track = value >= danger ? "#FEE2E2" : value >= warn ? "#F5ECE6" : "#E8E7EC";
  const pct = Math.min(100, value);

  return (
    <div className="flex flex-col gap-1.5">
      <div className="flex justify-between items-baseline">
        <span className="text-xs font-medium uppercase tracking-wider" style={{ color: "#9CA3AF" }}>
          {label}
        </span>
        <span className="text-sm font-bold tabular-nums" style={{ color, fontFamily: "JetBrains Mono, monospace" }}>
          {value}{unit}
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
