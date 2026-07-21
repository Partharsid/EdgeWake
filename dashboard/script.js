/* ============================================================
 *  EdgeWake — Forest Guardian Dashboard
 *  Interactive Logic & Simulation Engine
 * ============================================================ */

// ── State ──────────────────────────────────────────────────
let alertCount = 0;
let bootCount = 0;
let isSimulating = false;

// ── Clock ──────────────────────────────────────────────────
function updateClock() {
  const now = new Date();
  const h = String(now.getHours()).padStart(2, '0');
  const m = String(now.getMinutes()).padStart(2, '0');
  const s = String(now.getSeconds()).padStart(2, '0');
  document.getElementById('navClock').textContent = `${h}:${m}:${s}`;
}
setInterval(updateClock, 1000);
updateClock();

// ── Particle Background ────────────────────────────────────
(function initParticles() {
  const canvas = document.getElementById('particleCanvas');
  if (!canvas) return;
  const ctx = canvas.getContext('2d');

  let width, height, particles;

  function resize() {
    width = canvas.width = window.innerWidth;
    height = canvas.height = window.innerHeight;
  }

  function createParticles() {
    const count = Math.floor((width * height) / 18000);
    particles = [];
    for (let i = 0; i < count; i++) {
      particles.push({
        x: Math.random() * width,
        y: Math.random() * height,
        vx: (Math.random() - 0.5) * 0.3,
        vy: (Math.random() - 0.5) * 0.3,
        r: Math.random() * 1.5 + 0.5,
        alpha: Math.random() * 0.4 + 0.1,
      });
    }
  }

  function draw() {
    ctx.clearRect(0, 0, width, height);
    for (const p of particles) {
      p.x += p.vx;
      p.y += p.vy;
      if (p.x < 0) p.x = width;
      if (p.x > width) p.x = 0;
      if (p.y < 0) p.y = height;
      if (p.y > height) p.y = 0;

      ctx.beginPath();
      ctx.arc(p.x, p.y, p.r, 0, Math.PI * 2);
      ctx.fillStyle = `rgba(16, 185, 129, ${p.alpha})`;
      ctx.fill();
    }

    // Draw connections
    for (let i = 0; i < particles.length; i++) {
      for (let j = i + 1; j < particles.length; j++) {
        const dx = particles[i].x - particles[j].x;
        const dy = particles[i].y - particles[j].y;
        const dist = Math.sqrt(dx * dx + dy * dy);
        if (dist < 100) {
          ctx.beginPath();
          ctx.moveTo(particles[i].x, particles[i].y);
          ctx.lineTo(particles[j].x, particles[j].y);
          ctx.strokeStyle = `rgba(16, 185, 129, ${0.08 * (1 - dist / 100)})`;
          ctx.lineWidth = 0.5;
          ctx.stroke();
        }
      }
    }
    requestAnimationFrame(draw);
  }

  window.addEventListener('resize', () => {
    resize();
    createParticles();
  });

  resize();
  createParticles();
  draw();
})();

// ── Serial Monitor Logger ──────────────────────────────────
function serialLog(text, type = '') {
  const output = document.getElementById('serialOutput');
  const line = document.createElement('div');
  line.className = 'serial-line' + (type ? ` ${type}` : '');
  line.textContent = text;
  output.appendChild(line);
  output.scrollTop = output.scrollHeight;
}

function clearSerial() {
  document.getElementById('serialOutput').innerHTML = '';
  serialLog('[SERIAL] Monitor cleared.', 'dim');
}

// ── Cascade Tier Animation ─────────────────────────────────
function setTier(tierNum, mode = 'fire') {
  const steps = document.querySelectorAll('.tier-step');
  const connectors = document.querySelectorAll('.tier-connector');
  const tierClass = (tierNum >= 2) ? mode : 'active';

  steps.forEach((step, i) => {
    const t = i + 1;
    step.classList.remove('active', 'fire', 'vibration');
    if (t <= tierNum) {
      step.classList.add(tierClass);
    }
  });

  connectors.forEach((conn, i) => {
    conn.classList.remove('lit', 'fire-lit', 'vibration-lit');
    if (i + 1 < tierNum) {
      if (mode === 'vibration') {
        conn.classList.add('vibration-lit');
      } else if (mode === 'fire') {
        conn.classList.add('fire-lit');
      } else {
        conn.classList.add('lit');
      }
    }
  });
}

function resetTiers() {
  setTier(1, 'active');
}

// ── Update Device State Badge ──────────────────────────────
function setDeviceState(state) {
  const badge = document.getElementById('deviceStateBadge');
  badge.textContent = state;
  if (state === 'DEEP SLEEP') {
    badge.className = 'panel-badge sleep';
  } else {
    badge.className = 'panel-badge awake';
  }
}

// ── Add Alert to Feed ──────────────────────────────────────
function addAlertCard(type, location, deviceId) {
  const feed = document.getElementById('alertFeed');
  const empty = document.getElementById('emptyState');
  if (empty) empty.remove();

  alertCount++;
  document.getElementById('alertCount').textContent = alertCount;

  const now = new Date();
  const timeStr = now.toLocaleTimeString('en-US', { hour12: false });
  const dateStr = now.toLocaleDateString('en-US', { month: 'short', day: 'numeric' });

  const icons = {
    fire: '🔥',
    vibration: '📳',
    intruder: '🚨',
    other: 'ℹ️',
  };

  const titles = {
    fire: 'FIRE DETECTED',
    vibration: 'ABNORMAL VIBRATION DETECTED',
    intruder: 'INTRUDER DETECTED',
    other: 'GENERAL ALERT',
  };

  const card = document.createElement('div');
  card.className = `alert-card ${type}`;
  card.innerHTML = `
    <div class="alert-icon-wrap">${icons[type] || icons.other}</div>
    <div class="alert-body">
      <div class="alert-title">${titles[type] || titles.other}</div>
      <div class="alert-meta">
        <span>📍 ${location}</span>
        <span>🔧 ${deviceId}</span>
      </div>
      <div class="alert-time">${dateStr} · ${timeStr}</div>
    </div>
  `;

  feed.insertBefore(card, feed.firstChild);

  // Shake the stats card
  const statCard = document.getElementById('statAlerts');
  statCard.classList.add('shake');
  setTimeout(() => statCard.classList.remove('shake'), 600);
}

function clearAlerts() {
  const feed = document.getElementById('alertFeed');
  feed.innerHTML = `
    <div class="empty-state" id="emptyState">
      <svg width="48" height="48" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5" opacity="0.4">
        <path d="M12 22s8-4 8-10V5l-8-3-8 3v7c0 6 8 10 8 10z"/>
      </svg>
      <p>No alerts yet</p>
      <span>The forest is quiet. All nodes are in deep sleep.</span>
    </div>
  `;
  alertCount = 0;
  document.getElementById('alertCount').textContent = '0';
}

// ── Main Simulation: Full Tier Cascade ─────────────────────
// mode: 'fire' or 'vibration'
async function simulateAlert(mode = 'fire') {
  if (isSimulating) return;
  isSimulating = true;

  const btnFire = document.getElementById('btnSimulateFire');
  const btnVibration = document.getElementById('btnSimulateVibration');
  btnFire.disabled = true;
  btnVibration.disabled = true;

  const statusDot = document.getElementById('systemStatusDot');
  const statusText = document.getElementById('systemStatusText');

  const isFire = (mode === 'fire');
  const alertLabel = isFire ? 'Fire' : 'Vibration';
  const triggerEmoji = isFire ? '🔥' : '📳';
  const triggerPin = isFire ? 'GPIO 13' : 'GPIO 2';
  const triggerName = isFire ? 'Flame / smoke sensor' : 'Vibration sensor (SW-420)';

  bootCount++;
  document.getElementById('infoBootCount').textContent = bootCount;

  // Helper: wait
  const wait = (ms) => new Promise(r => setTimeout(r, ms));

  // ── TIER 2: Hardware Interrupt ──
  serialLog('');
  serialLog('╔══════════════════════════════════════════╗', 'warn');
  serialLog(`║      🌲  EdgeWake — Forest Guard  🌲     ║`, 'warn');
  serialLog('╠══════════════════════════════════════════╣', 'warn');
  serialLog(`║  Boot #${bootCount}  |  Device: EDGEWAKE-001       ║`, 'warn');
  serialLog('╚══════════════════════════════════════════╝', 'warn');
  await wait(300);

  serialLog('[WAKE] Cause: EXT1 (Multi-Sensor Interrupt)');
  setTier(2, mode);
  setDeviceState('AWAKE — TIER 2');
  statusDot.classList.add('alert');
  statusText.textContent = 'ALERT ACTIVE';
  await wait(500);

  serialLog(`${triggerEmoji} [TIER 2] HARDWARE INTERRUPT DETECTED!`, 'error');
  serialLog(`[TIER 2] ${triggerName} triggered on ${triggerPin}.`, 'warn');
  if (!isFire) {
    serialLog('[TIER 2] Abnormal vibration or sound detected!', 'warn');
  }
  await wait(600);

  // ── TIER 3: Audio Verification ──
  setTier(3, mode);
  setDeviceState('AWAKE — TIER 3');
  serialLog('🎙️ [TIER 3] Starting audio verification...', 'info');
  serialLog('[MIC] Initialising I2S...', 'dim');
  await wait(400);
  serialLog('[MIC] Initialised OK.', 'dim');
  serialLog('[MIC] Recording audio...', 'dim');
  await wait(1200);

  const fakeRms = (Math.random() * 1000 + 2000).toFixed(1);
  serialLog(`[MIC] Recorded 32000 samples (2.0 sec)`, 'dim');
  serialLog(`[AUDIO] RMS energy = ${fakeRms}  (threshold = 1500)`, 'info');
  serialLog('[AUDIO] ✓ Threat VERIFIED by audio analysis.');
  serialLog('[MIC] Driver uninstalled.', 'dim');
  await wait(500);

  // ── TIER 4: Visual Capture ──
  setTier(4, mode);
  setDeviceState('AWAKE — TIER 4');
  serialLog('📷 [TIER 4] Initialising camera for visual capture...', 'info');
  serialLog('[CAM] PSRAM detected — using high-res capture.', 'dim');
  serialLog('[CAM] Initialised OK.', 'dim');
  await wait(400);
  serialLog('[CAM] Capturing photo...');
  await wait(600);

  const fakeSize = Math.floor(Math.random() * 30000 + 40000);
  serialLog(`[CAM] Captured ${fakeSize} bytes (800x600)`);
  await wait(400);

  // ── TIER 5: Cloud Handoff ──
  setTier(5, mode);
  setDeviceState('AWAKE — TIER 5');
  serialLog('☁️ [TIER 5] Connecting to Wi-Fi for cloud handoff...', 'info');
  serialLog('[NET] Connecting to \'ForestNetwork\'....', 'dim');
  await wait(800);

  serialLog('[NET] Connected!  IP: 192.168.1.42  RSSI: -62 dBm');
  document.getElementById('infoRssi').textContent = '-62 dBm';
  await wait(300);

  serialLog(`[NET] Sending alert (type: ${alertLabel}) to Telegram...`);
  await wait(700);
  serialLog('[NET] Server responded: 200');
  serialLog('[MAIN] ✅ Alert sent successfully!');
  await wait(300);

  serialLog('[NET] Wi-Fi OFF.', 'dim');

  // Update last trigger time
  const triggerTime = new Date().toLocaleTimeString('en-US', { hour12: false });
  document.getElementById('infoLastTrigger').textContent = triggerTime;

  // Add alert card
  addAlertCard(mode, 'Forest Sector A', 'EDGEWAKE-001');

  await wait(500);

  // ── Back to sleep ──
  serialLog('[MAIN] Full cycle complete. Returning to deep sleep...');
  serialLog('💤 [SLEEP] Configuring ext1 wake-up on GPIO 13 + GPIO 2...', 'dim');
  serialLog('💤 [SLEEP] Entering deep sleep... goodnight. 🌙', 'dim');

  await wait(800);
  resetTiers();
  setDeviceState('DEEP SLEEP');
  statusDot.classList.remove('alert');
  statusText.textContent = 'System Online';

  // Decrease fake battery
  const battEl = document.getElementById('batteryValue');
  let batt = parseInt(battEl.textContent);
  batt = Math.max(5, batt - Math.floor(Math.random() * 3 + 1));
  battEl.textContent = batt + '%';

  isSimulating = false;
  btnFire.disabled = false;
  btnVibration.disabled = false;
}

// ── Initialize ─────────────────────────────────────────────
document.addEventListener('DOMContentLoaded', () => {
  resetTiers();
  serialLog('[SYS] Dashboard loaded. Waiting for device events...', 'dim');
});
