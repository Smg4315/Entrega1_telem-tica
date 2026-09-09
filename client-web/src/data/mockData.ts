export type NodeStatus = "active" | "sending" | "disconnected" | "registering";

export interface NodeMetric {
  timestamp: string;
  cpu: number;
  mem: number;
  temp: number;
  bat: number;
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
  current: { cpu: number; mem: number; temp: number; bat: number };
  history: NodeMetric[];
  events: NodeEvent[];
  lastSeen: string;
}

function hist(base: { cpu: number; mem: number; temp: number; bat: number }): NodeMetric[] {
  const now = Date.now();
  return Array.from({ length: 10 }, (_, i) => {
    const t = new Date(now - (9 - i) * 60_000);
    const jitter = (n: number, r: number) => Math.max(0, Math.min(100, n + (Math.random() - 0.5) * r));
    return {
      timestamp: t.toLocaleTimeString("es-CO", { hour: "2-digit", minute: "2-digit" }),
      cpu: Math.round(jitter(base.cpu, 14)),
      mem: Math.round(jitter(base.mem, 10)),
      temp: Math.round(jitter(base.temp, 6)),
      bat: Math.round(jitter(base.bat, 4)),
    };
  });
}

export const mockNodes: MonitoredNode[] = [
  {
    id: "NODE01",
    label: "Servidor Web Principal",
    location: "Rack A — Sala 1",
    status: "active",
    current: { cpu: 47, mem: 61, temp: 39, bat: 92 },
    history: hist({ cpu: 47, mem: 61, temp: 39, bat: 92 }),
    lastSeen: "hace 8 s",
    events: [
      { id: "E001", timestamp: "10:14:32", type: "HIGH_TEMP", message: "Temperatura alcanzó 88 °C — ventilador activado" },
      { id: "E002", timestamp: "09:52:11", type: "RECONNECT", message: "Nodo reconectado tras pérdida de red" },
    ],
  },
  {
    id: "NODE02",
    label: "Base de Datos Primaria",
    location: "Rack B — Sala 1",
    status: "sending",
    current: { cpu: 72, mem: 84, temp: 51, bat: 100 },
    history: hist({ cpu: 72, mem: 84, temp: 51, bat: 100 }),
    lastSeen: "hace 3 s",
    events: [
      { id: "E003", timestamp: "10:22:05", type: "HIGH_TEMP", message: "TEMP=91 °C — umbral crítico superado" },
    ],
  },
  {
    id: "NODE03",
    label: "Balanceador de Carga",
    location: "Rack A — Sala 2",
    status: "active",
    current: { cpu: 28, mem: 43, temp: 32, bat: 78 },
    history: hist({ cpu: 28, mem: 43, temp: 32, bat: 78 }),
    lastSeen: "hace 12 s",
    events: [],
  },
  {
    id: "NODE04",
    label: "Nodo Edge IoT",
    location: "Exterior — Edificio B",
    status: "disconnected",
    current: { cpu: 0, mem: 0, temp: 0, bat: 14 },
    history: hist({ cpu: 18, mem: 32, temp: 28, bat: 18 }),
    lastSeen: "hace 4 min",
    events: [
      { id: "E004", timestamp: "10:19:43", type: "DISCONNECT", message: "Pérdida de conexión — sin respuesta ACK" },
      { id: "E005", timestamp: "10:19:43", type: "LOW_BAT", message: "BAT=14 — batería crítica antes de desconexión" },
    ],
  },
  {
    id: "NODE05",
    label: "Servidor de Archivos",
    location: "Rack C — Sala 2",
    status: "active",
    current: { cpu: 19, mem: 55, temp: 35, bat: 100 },
    history: hist({ cpu: 19, mem: 55, temp: 35, bat: 100 }),
    lastSeen: "hace 6 s",
    events: [],
  },
  {
    id: "NODE06",
    label: "Sensor Ambiental",
    location: "Rack C — Sala 2",
    status: "registering",
    // Registrando: REGISTER enviado, esperando ACK. Aún no ha enviado STATUS,
    // por lo que no hay métricas ni histórico reales todavía (docx, sección 6 — Máquinas de Estado).
    current: { cpu: 0, mem: 0, temp: 0, bat: 0 },
    history: [],
    lastSeen: "registro enviado hace 2 s",
    events: [],
  },
];

export const mockProtocolLog = [
  { time: "10:23:01", dir: "←", src: "CLIENT", msg: "QUERY|099|NODE99|CURRENT" },
  { time: "10:23:01", dir: "→", src: "SERVER", msg: "ERROR|099|UNKNOWN_NODE|NODE99" },
  { time: "10:22:47", dir: "→", src: "NODE06", msg: "REGISTER|001|NODE06" },
  { time: "10:22:47", dir: "←", src: "SERVER", msg: "ACK|001|REGISTER|NODE06" },
  { time: "10:22:18", dir: "→", src: "NODE02", msg: "EVENT|043|NODE02|HIGH_TEMP|TEMP=91" },
  { time: "10:22:18", dir: "←", src: "SERVER", msg: "ACK|043|EVENT|NODE02" },
  { time: "10:22:10", dir: "→", src: "NODE01", msg: "STATUS|312|NODE01|CPU=47|MEM=61|TEMP=39|BAT=92" },
  { time: "10:22:10", dir: "←", src: "SERVER", msg: "ACK|312|STATUS|NODE01" },
  { time: "10:22:05", dir: "←", src: "CLIENT", msg: "QUERY|101|NODE02|CURRENT" },
  { time: "10:22:05", dir: "→", src: "SERVER", msg: "RESPONSE|101|NODE02|CURRENT|CPU=72|MEM=84|TEMP=51|BAT=100" },
  { time: "10:19:43", dir: "→", src: "NODE04", msg: "EVENT|038|NODE04|DISCONNECT" },
  { time: "10:15:00", dir: "→", src: "NODE03", msg: "STATUS|289|NODE03|CPU=28|MEM=43|TEMP=32|BAT=78" },
  { time: "10:15:00", dir: "←", src: "SERVER", msg: "ACK|289|STATUS|NODE03" },
  { time: "10:14:32", dir: "→", src: "NODE01", msg: "EVENT|031|NODE01|HIGH_TEMP|TEMP=88" },
  { time: "10:14:32", dir: "←", src: "SERVER", msg: "ACK|031|EVENT|NODE01" },
];
