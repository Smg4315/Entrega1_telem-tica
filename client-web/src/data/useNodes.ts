import { useEffect, useState } from "react";
import { FETCH_TIMEOUT_MS, GATEWAY_URL, KNOWN_NODES, POLL_INTERVAL_MS } from "../config";
import type { LogEntry, MonitoredNode, NodeMetric } from "./types";

const HISTORY_LEN = 10;
const LOG_LEN = 40;

const NO_METRICS = { cpu: null, mem: null, temp: null, bat: null };

const ERROR_TEXT: Record<string, string> = {
  UNKNOWN_NODE: "Nodo no registrado en el servidor",
  SERVER_UNREACHABLE: "El gateway no pudo conectar con el servidor NMP",
  GATEWAY_UNREACHABLE: "Sin conexión con el gateway",
  INVALID_REPLY: "El servidor NMP devolvió una respuesta ilegible",
};

// Respuesta de GET /nodos/{id}/historico (gateway/app/http/query_routes.py).
interface GatewayReply {
  status?: "active" | "disconnected";
  metrics?: Record<string, string>;
  error?: string;
  nmp?: { request: string; reply?: string };
}

type QueryResult =
  | { ok: true; status: "active" | "disconnected"; metrics: Record<string, string>; nmp?: GatewayReply["nmp"] }
  | { ok: false; code: string; nmp?: GatewayReply["nmp"] };

async function queryNode(nodeId: string): Promise<QueryResult> {
  let res: Response;
  let body: GatewayReply;
  try {
    res = await fetch(`${GATEWAY_URL}/nodos/${nodeId}/historico`, {
      signal: AbortSignal.timeout(FETCH_TIMEOUT_MS),
    });
    body = await res.json();
  } catch {
    // Gateway caído, timeout, CORS o cuerpo que no es JSON.
    return { ok: false, code: "GATEWAY_UNREACHABLE" };
  }

  if (!res.ok || body.error || !body.status) {
    return { ok: false, code: body.error ?? `HTTP_${res.status}`, nmp: body.nmp };
  }
  return { ok: true, status: body.status, metrics: body.metrics ?? {}, nmp: body.nmp };
}

function metric(metrics: Record<string, string>, key: string): number | null {
  const n = Number(metrics[key]);
  return metrics[key] !== undefined && Number.isFinite(n) ? n : null;
}

function applyResult(node: MonitoredNode, result: QueryResult, time: string): MonitoredNode {
  let current = NO_METRICS as MonitoredNode["current"];
  let next: Pick<MonitoredNode, "status" | "lastSeen" | "error">;

  if (!result.ok) {
    next = {
      status: "error",
      error: ERROR_TEXT[result.code] ?? `Error: ${result.code}`,
      lastSeen: node.lastSeen,
    };
  } else {
    current = {
      cpu: metric(result.metrics, "CPU"),
      mem: metric(result.metrics, "MEM"),
      temp: metric(result.metrics, "TEMP"),
      bat: metric(result.metrics, "BAT"),
    };
    next = { status: result.status, error: undefined, lastSeen: time };
  }

  // Un punto por sondeo en todos los nodos, para que los históricos queden
  // alineados; sin lectura vigente (error o desconectado) el punto va vacío.
  const point: NodeMetric = {
    timestamp: time,
    ...(result.ok && result.status === "active" ? current : NO_METRICS),
  };

  return { ...node, ...next, current, history: [...node.history, point].slice(-HISTORY_LEN) };
}

function logEntries(nodeId: string, result: QueryResult, time: string): LogEntry[] {
  const entries: LogEntry[] = [];
  // Más reciente primero: la respuesta va antes que su solicitud.
  if (result.nmp?.reply) {
    entries.push({ time, dir: "→", src: "SERVER", msg: result.nmp.reply });
  } else if (!result.ok) {
    entries.push({ time, dir: "→", src: "GATEWAY", msg: `ERROR: ${result.code} (${nodeId})` });
  }
  if (result.nmp?.request) {
    entries.push({ time, dir: "←", src: "CLIENT", msg: result.nmp.request });
  }
  return entries;
}

const INITIAL_NODES: MonitoredNode[] = KNOWN_NODES.map(n => ({
  ...n,
  status: "loading",
  current: NO_METRICS,
  history: [],
  events: [], // el gateway no expone eventos todavía
  lastSeen: "—",
}));

/** Consulta cada nodo de KNOWN_NODES al montar y luego cada POLL_INTERVAL_MS. */
export function useNodes(enabled: boolean): { nodes: MonitoredNode[]; log: LogEntry[] } {
  const [nodes, setNodes] = useState(INITIAL_NODES);
  const [log, setLog] = useState<LogEntry[]>([]);

  useEffect(() => {
    if (!enabled) return;
    let cancelled = false;

    async function poll() {
      const results = await Promise.all(KNOWN_NODES.map(n => queryNode(n.id)));
      if (cancelled) return;

      const time = new Date().toLocaleTimeString("es-CO", { hour12: false });
      setNodes(prev => prev.map((node, i) => applyResult(node, results[i], time)));
      setLog(prev =>
        [...results.flatMap((r, i) => logEntries(KNOWN_NODES[i].id, r, time)), ...prev].slice(0, LOG_LEN),
      );
    }

    poll();
    const timer = setInterval(poll, POLL_INTERVAL_MS);
    return () => {
      cancelled = true;
      clearInterval(timer);
    };
  }, [enabled]);

  return { nodes, log };
}
