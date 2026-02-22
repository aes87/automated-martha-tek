/**
 * app.js — Martha Tent Controller Dashboard
 *
 * Connects to the ESP32 WebSocket (/ws) for live sensor data (2s push).
 * Fetches initial config from /api/config on load.
 * Manages relay manual override controls.
 */

'use strict';

// ── Constants ─────────────────────────────────────────────────────────────────
const RELAY_NAMES  = ['Fogger','TubFan','Exhaust','Intake','UVC','Lights','Pump','Spare'];
const CHART_POINTS = 900;  // 30 min × 2s intervals
const WS_RECONNECT_MS = 3000;

// VPD calculation (mirrors firmware vpd.h)
function calcSVP(tc) { return 0.6108 * Math.exp(17.27 * tc / (tc + 237.3)); }
function calcVPD(tc, rh) { return calcSVP(tc) * (1 - rh / 100); }

// ── State ─────────────────────────────────────────────────────────────────────
let ws         = null;
let isManual   = false;
let isArmed    = false;
let latestSnap = null;

// ── Chart setup ───────────────────────────────────────────────────────────────
const CHART_DEFAULTS = {
  type: 'line',
  options: {
    animation: false,
    responsive: true,
    maintainAspectRatio: false,
    plugins: { legend: { display: false } },
    scales: {
      x: { display: false },
      y: {
        display: true,
        grid:   { color: 'rgba(255,255,255,0.05)' },
        ticks:  { color: '#64748b', font: { size: 9 } },
      }
    },
    elements: { point: { radius: 0 }, line: { tension: 0.3 } }
  }
};

function makeBuffer() {
  return Array(CHART_POINTS).fill(null);
}

function makeChart(canvasId, color) {
  const ctx = document.getElementById(canvasId).getContext('2d');
  const buf = makeBuffer();
  const chart = new Chart(ctx, {
    ...CHART_DEFAULTS,
    data: {
      labels: buf.map((_, i) => i),
      datasets: [{ data: buf, borderColor: color, borderWidth: 1.5, fill: false }]
    }
  });
  return { chart, buf };
}

const charts = {
  co2: makeChart('chart-co2', '#22d3ee'),
  rh:  makeChart('chart-rh',  '#22c55e'),
};

function pushChart(key, value) {
  const { chart, buf } = charts[key];
  buf.shift(); buf.push(value);
  chart.data.datasets[0].data = [...buf];
  chart.update('none');
}

// ── Relay grid ────────────────────────────────────────────────────────────────
function buildRelayGrid() {
  const grid = document.getElementById('relay-grid');
  grid.innerHTML = '';
  RELAY_NAMES.forEach((name, i) => {
    const card = document.createElement('div');
    card.className = 'relay-card';
    card.id = `relay-${i}`;
    card.innerHTML = `<div class="relay-dot"></div><span class="relay-name">${name}</span>`;
    card.addEventListener('click', () => toggleRelay(i, name));
    grid.appendChild(card);
  });
}

async function toggleRelay(idx, name) {
  if (!isArmed || !isManual) return;
  const card = document.getElementById(`relay-${idx}`);
  const currentlyOn = card.classList.contains('on');
  try {
    const res = await fetch(`/api/relay/${name}/set`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ state: !currentlyOn })
    });
    if (!res.ok) console.warn(`Relay ${name} toggle failed`);
  } catch (e) {
    console.error('Relay toggle error:', e);
  }
}

function updateRelayCard(idx, on, armed, manual) {
  const card = document.getElementById(`relay-${idx}`);
  if (!card) return;
  card.classList.toggle('on', on);
  card.classList.toggle('locked', !manual || !armed);
  card.title = !armed ? 'Boot locked' : !manual ? 'Enter manual mode to control' : (on ? 'Click to turn OFF' : 'Click to turn ON');
}

// ── WebSocket ─────────────────────────────────────────────────────────────────
function connectWs() {
  const url = `ws://${location.host}/ws`;
  ws = new WebSocket(url);

  ws.onopen = () => {
    document.getElementById('conn-status').className = 'conn-dot connected';
  };

  ws.onclose = () => {
    document.getElementById('conn-status').className = 'conn-dot disconnected';
    setTimeout(connectWs, WS_RECONNECT_MS);
  };

  ws.onerror = () => ws.close();

  ws.onmessage = (evt) => {
    try {
      const d = JSON.parse(evt.data);
      latestSnap = d;
      updateUI(d);
    } catch (e) {
      console.error('WS parse error:', e);
    }
  };
}

function updateUI(d) {
  // CO2
  const co2 = d.co2 ?? 0;
  document.getElementById('co2-val').textContent = co2.toFixed(0);
  document.getElementById(`card-co2`).style.borderColor = co2 > 950 ? '#ef4444' : '';
  pushChart('co2', co2);

  // RH
  const rh = d.rh_a ?? 0;
  document.getElementById('rh-val').textContent = rh.toFixed(1);
  document.getElementById(`card-rh`).style.borderColor = rh < 80 ? '#ef4444' : '';
  pushChart('rh', rh);

  // Water
  const wl = d.wl ?? 0;
  document.getElementById('water-val').textContent = wl.toFixed(0);
  document.getElementById('card-water').style.borderColor = wl < 20 ? '#ef4444' : '';

  // VPD (use first shelf sensor for temp)
  const rhArr  = d.rh  ?? [0, 0, 0];
  const tpArr  = d.tp  ?? [0, 0, 0, 0, 0];
  const vpd    = calcVPD(tpArr[0] || 22, rhArr[0] || rh);
  document.getElementById('vpd-val').textContent = vpd.toFixed(2);

  // Shelf cells
  for (let i = 0; i < 3; ++i) {
    const cell = document.getElementById(`shelf-${i}`);
    if (!cell) continue;
    cell.querySelector('.shelf-rh').textContent   = `${(rhArr[i] ?? 0).toFixed(1)}% RH`;
    cell.querySelector('.shelf-temp').textContent = `${(tpArr[i] ?? 0).toFixed(1)} °C`;
    cell.querySelector('.shelf-probe').textContent= `${(tpArr[i] ?? 0).toFixed(1)} °C (substrate)`;
  }

  // Relay states
  const mask   = d.rl  ?? 0;
  isArmed  = d.am  ?? false;
  isManual = d.mm  ?? false;

  for (let i = 0; i < 8; ++i) {
    updateRelayCard(i, !!(mask & (1 << i)), isArmed, isManual);
  }

  // Manual mode badge + buttons
  const badge = document.getElementById('relay-mode-badge');
  badge.textContent = isManual ? 'MANUAL' : '';
  badge.classList.toggle('visible', isManual);
  document.getElementById('btn-manual-on').style.display  = isManual ? 'none' : '';
  document.getElementById('btn-manual-off').style.display = isManual ? '' : 'none';

  // Uptime
  const ut = d.t ?? 0;
  const h  = Math.floor(ut / 3600000);
  const m  = Math.floor((ut % 3600000) / 60000);
  const s  = Math.floor((ut % 60000) / 1000);
  document.getElementById('uptime').textContent =
    `uptime: ${h}h ${m}m ${s}s`;
}

// ── Config panel ──────────────────────────────────────────────────────────────
function minuteToTime(min) {
  const h = String(Math.floor(min / 60)).padStart(2, '0');
  const m = String(min % 60).padStart(2, '0');
  return `${h}:${m}`;
}

function timeToMinute(str) {
  const [h, m] = str.split(':').map(Number);
  return h * 60 + m;
}

async function loadConfig() {
  try {
    const res = await fetch('/api/config');
    const cfg = await res.json();
    document.getElementById('cfg-rh-on').value    = cfg.rh_on_pct    ?? 85;
    document.getElementById('cfg-rh-hyst').value  = cfg.rh_hysteresis ?? 2;
    document.getElementById('cfg-co2-on').value   = cfg.co2_on_ppm   ?? 950;
    document.getElementById('cfg-co2-off').value  = cfg.co2_off_ppm  ?? 800;
    document.getElementById('cfg-lights-on').value  = minuteToTime(cfg.timer?.lights_on_minute  ?? 480);
    document.getElementById('cfg-lights-off').value = minuteToTime(cfg.timer?.lights_off_minute ?? 1200);
    document.getElementById('cfg-uvc-on').value   = cfg.timer?.uvc_on_min  ?? 60;
    document.getElementById('cfg-uvc-off').value  = cfg.timer?.uvc_off_min ?? 240;
    document.getElementById('fw-ver').textContent = '';  // pulled from status endpoint
  } catch (e) {
    console.warn('Config load failed:', e);
  }
}

async function saveConfig() {
  const body = {
    rh_on_pct:     parseFloat(document.getElementById('cfg-rh-on').value),
    rh_hysteresis: parseFloat(document.getElementById('cfg-rh-hyst').value),
    co2_on_ppm:    parseFloat(document.getElementById('cfg-co2-on').value),
    co2_off_ppm:   parseFloat(document.getElementById('cfg-co2-off').value),
    timer: {
      lights_on_minute:  timeToMinute(document.getElementById('cfg-lights-on').value),
      lights_off_minute: timeToMinute(document.getElementById('cfg-lights-off').value),
      uvc_on_min:        parseInt(document.getElementById('cfg-uvc-on').value),
      uvc_off_min:       parseInt(document.getElementById('cfg-uvc-off').value),
    }
  };

  const status = document.getElementById('cfg-status');
  try {
    const res = await fetch('/api/config', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(body)
    });
    if (res.ok) {
      status.textContent = 'Saved.';
      status.className   = 'cfg-status ok';
    } else {
      const err = await res.json();
      status.textContent = `Error: ${err.error ?? res.status}`;
      status.className   = 'cfg-status err';
    }
  } catch (e) {
    status.textContent = `Network error: ${e.message}`;
    status.className   = 'cfg-status err';
  }
  setTimeout(() => { status.textContent = ''; }, 3000);
}

// ── Manual mode buttons ───────────────────────────────────────────────────────
async function setManualMode(enable) {
  await fetch('/api/relay/manual', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ manual: enable })
  });
}

// ── Init ──────────────────────────────────────────────────────────────────────
document.addEventListener('DOMContentLoaded', () => {
  buildRelayGrid();
  connectWs();
  loadConfig();

  document.getElementById('btn-config-save').addEventListener('click', saveConfig);
  document.getElementById('btn-manual-on').addEventListener('click',  () => setManualMode(true));
  document.getElementById('btn-manual-off').addEventListener('click', () => setManualMode(false));
});
