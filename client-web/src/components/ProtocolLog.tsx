import type { LogEntry } from "../data/types";

export default function ProtocolLog({ entries }: { entries: LogEntry[] }) {
  return (
    <div className="flex flex-col h-full bg-white" style={{ borderLeft: "1px solid #E5E7EB" }}>
      <div className="px-4 py-4 shrink-0" style={{ borderBottom: "1px solid #E5E7EB" }}>
        <div className="text-xs font-bold uppercase tracking-wider" style={{ color: "#403662" }}>Log NMP</div>
        <div className="text-xs mt-0.5" style={{ color: "#9CA3AF" }}>Protocolo de Aplicación</div>
      </div>

      <div className="flex-1 overflow-auto p-3 flex flex-col gap-1.5">
        {entries.length === 0 && (
          <div className="text-xs px-1 py-2" style={{ color: "#9CA3AF" }}>Sin mensajes todavía</div>
        )}
        {entries.map((entry, i) => {
          const isSend = entry.dir === "→";
          const isError = entry.msg.startsWith("ERROR");
          const isEvent = entry.msg.includes("|EVENT|") || entry.msg.includes("DISCONNECT");
          const color = isError ? "#EF4444" : isEvent ? "#BE8156" : isSend ? "#6F828A" : "#403662";

          return (
            <div key={i} className="rounded-lg px-3 py-2 text-xs" style={{ background: "#F9FAFB", fontFamily: "JetBrains Mono, monospace" }}>
              <div className="flex items-center gap-1.5 mb-0.5">
                <span style={{ color: "#9CA3AF" }}>{entry.time}</span>
                <span style={{ color }} className="font-bold">{entry.dir}</span>
                <span className="font-semibold" style={{ color: "#6B7280" }}>{entry.src}</span>
              </div>
              <div className="text-xs break-all" style={{ color: isError ? "#EF4444" : isEvent ? "#B45309" : "#4B5563" }}>
                {entry.msg}
              </div>
            </div>
          );
        })}
      </div>

      <div className="px-4 py-3 shrink-0 text-xs" style={{ borderTop: "1px solid #E5E7EB", color: "#D1D5DB" }}>
        TCP/UDP · Sockets Berkeley
      </div>
    </div>
  );
}
