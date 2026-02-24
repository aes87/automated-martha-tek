# Chapter 08 — Low-Voltage Signal Wiring (ESP32 → Relay)

**What you'll do:** Wire the ESP32-S3 GPIO pins to the relay board IN pins, route
through the failsafe panel topology, label every wire, and verify continuity.

**Prerequisites:** Chapters 04–07 complete. 22 AWG stranded wire, label maker,
multimeter.

> ⚠️ **SAFETY:** All wiring here is low-voltage (3.3V logic signals). Enclosure
> unplugged.

---

## Understanding the Signal Path

Each relay IN pin receives a signal from the ESP32 and also from the failsafe
group switch. Both connect to the same IN pin — the 10 kΩ pull-up holds the pin
HIGH (relay OFF) when neither is actively driving it LOW.

```
ESP32 GPIO ──────────────────────────── Relay INx
                            ↑
DPDT common wire → SPST group switch ──┘  (active in MANUAL only)
```

In AUTO mode the ESP32 GPIO drives each IN pin directly. In MANUAL mode the group
switch drives it. There is no interrupt or changeover — both sources connect to the
same IN pin at all times. The pull-up ensures a safe default when both are idle.

---

## GPIO-to-Relay Channel Mapping

| ESP32 GPIO | Relay IN pin | Load |
|-----------|-------------|------|
| `GPIO 38` | IN1 | Fogger |
| `GPIO 39` | IN2 | Tub fan |
| `GPIO 18` | IN3 | Exhaust fan |
| `GPIO 19` | IN4 | Intake fan |
| `GPIO 40` | IN5 | UVC lights |
| `GPIO 41` | IN6 | Grow lights |
| `GPIO 42` | IN7 | Pump (pre-wired for optional add-on) |
| `GPIO 47` | IN8 | Spare |

> **[?] Active LOW:** The relay fires when its IN pin is pulled to GND (LOW). It is
> OFF when the IN pin is HIGH (3.3V). Firmware uses `digitalWrite(pin, LOW)` to fire
> and `digitalWrite(pin, HIGH)` to release.

---

## Step 1 — Measure, Cut, and Label Wires

1. Route a wire from the ESP32 GPIO header to relay IN1 without connecting — measure
   the length and add 5 cm for strain relief. Cut 8 wires at this length.
2. Strip 5 mm from each end. Crimp ferrules on all ends.
3. Before connecting anything, label each wire at both ends:
   - One end: `GPIO 38` (or the GPIO number)
   - Other end: `IN1` (or the relay IN number)

---

## Step 2 — Connect GPIOs to IN Pins

Work through all 8 channels. Insert each wire end into the ESP32 GPIO header
(Dupont female-to-female connectors work well here) and the relay board IN pin:

1. `GPIO 38` → `IN1`
2. `GPIO 39` → `IN2`
3. `GPIO 18` → `IN3`
4. `GPIO 19` → `IN4`
5. `GPIO 40` → `IN5`
6. `GPIO 41` → `IN6`
7. `GPIO 42` → `IN7`
8. `GPIO 47` → `IN8`

Route wires through the LV zone only — do not cross the mains zone or run parallel
to mains wiring. Bundle the 8 wires with cable ties.

---

## Step 3 — Connect Relay Board VCC and GND

| Relay board pin | Connect to |
|----------------|-----------|
| VCC (optocoupler logic supply) | ESP32 `3V3` |
| JD-VCC (relay coil supply) | 5V PSU positive output |
| GND | Common GND |

> ⚠️ **SAFETY:** VCC and JD-VCC are now separate supplies (jumper removed in Ch 04).
> VCC → 3.3V; JD-VCC → 5V. Do not connect JD-VCC to 3.3V — relay coils are rated
> for 5V; undersupplying them causes unreliable contact closure under load.

---

## Step 4 — Verify Continuity

1. Continuity mode — for each of the 8 channels: continuity between the ESP32 GPIO
   pad and the relay IN pin through the wire.
2. No continuity between any adjacent IN pins (would indicate a solder bridge or
   misrouted wire).
3. Continuity: relay VCC → ESP32 3V3.
4. Continuity: relay GND → ESP32 GND.

**✓ Check:** All 8 GPIO-to-IN paths pass continuity. No cross-channel shorts.
VCC and GND confirmed.

---

## Step 5 — Re-verify Failsafe Integration

With ESP32 GPIO wires now added to the IN pins:

1. Disconnect the ESP32 USB (GPIOs float to HIGH via pull-ups).
2. DPDT to MANUAL. Close HUMIDITY switch.
3. IN1 to GND: continuity (manual switch wins over floating GPIO).
4. Open HUMIDITY: IN1 returns to no-continuity.

**✓ Check:** Failsafe switches still operate correctly with GPIO wires attached.

---

## Chapter 08 Checkpoint

- [ ] All 8 signal wires cut to length, labelled at both ends
- [ ] GPIO 38 → IN1 (Fogger)
- [ ] GPIO 39 → IN2 (Tub fan)
- [ ] GPIO 18 → IN3 (Exhaust fan)
- [ ] GPIO 19 → IN4 (Intake fan)
- [ ] GPIO 40 → IN5 (UVC lights)
- [ ] GPIO 41 → IN6 (Grow lights)
- [ ] GPIO 42 → IN7 (Pump — pre-wired)
- [ ] GPIO 47 → IN8 (Spare)
- [ ] Relay VCC → ESP32 3V3
- [ ] Relay JD-VCC → 5V PSU output
- [ ] Relay GND → common GND
- [ ] Continuity: all 8 GPIO-to-IN paths pass; no cross-channel shorts
- [ ] Failsafe re-verification passed

---

[← Ch 07 — Failsafe Panel Switches](07-failsafe-panel.md)  ·  [Ch 09 — Mains Wiring →](09-mains-wiring.md)
