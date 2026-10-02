// "loading": aún no llega la primera respuesta. "error": la consulta falló.
export type NodeStatus = "active" | "sending" | "disconnected" | "registering" | "loading" | "error";

// null = el servidor no tiene esa métrica (o no hubo lectura en ese sondeo).
export interface NodeMetric {
  timestamp: string;
  cpu: number | null;
  mem: number | null;
  temp: number | null;
  bat: number | null;
}

export interface NodeEvent {
  id: string;
  timestamp: string;
  type: "HIGH_TEMP" | "LOW_BAT" | "DISCONNECT" | "RECONNECT" | "FAULT";
  message: string;
}

export interface MonitoredNode {
  id: string;
  label: string;
  location: string;
  status: NodeStatus;
  current: Omit<NodeMetric, "timestamp">;
  history: NodeMetric[];
  events: NodeEvent[];
  lastSeen: string;
  error?: string; // mensaje para mostrar cuando status === "error"
}

export interface LogEntry {
  time: string;
  dir: "→" | "←";
  src: string;
  msg: string;
}
