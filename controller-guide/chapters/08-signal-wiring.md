# Chapter 08 — Low-Voltage Signal Wiring (ESP32 → Relay)

**What you'll do:** Wire the ESP32-S3 GPIO pins to the relay board IN pins,
routed through the failsafe panel topology from Chapter 07. Label every wire.
Verify continuity before powering on.

**Prerequisites:** Chapters 04–07 complete (relay board with pull-ups mounted,
ESP32 mounted, failsafe panel wired). 22 AWG stranded wire, label maker,
multimeter.

> ⚠️ **SAFETY:** All wiring in this chapter is low-voltage (3.3V logic signals).
> Keep the enclosure unplugged.

---

## Understanding the Signal Path

Each relay channel's IN pin receives a signal from the ESP32, routed through the
failsafe topology:

```
ESP32 GPIO ──────────────────────────────────────────► Relay INx
                                           ↑
               DPDT common wire ── (connected in MANUAL: GND)
               SPST group switch ──────────┘
```

In AUTO mode, the ESP32 GPIO is the direct driver of each IN pin. In MANUAL mode,
the group switch also connects to the same IN pin — because the IN pin is always
connected to both the ESP32 GPIO and the manual switch output. The 10 kΩ pull-up
on each IN pin prevents floating when neither is driving.

**There is nothing to change or interrupt in the ESP32 → IN wiring path.** The
failsafe topology from Chapter 07 already connects to each IN pin. In this chapter,
you are simply adding the ESP32 GPIO wire to the same IN pins.

---

## GPIO-to-Relay Channel Mapping

| ESP32 GPIO | Relay IN pin | Load | Wire colour (suggest) |
|-----------|-------------|------|-----------------------|
| `GPIO 38` | IN1 | Fogger | Red |
| `GPIO 39` | IN2 | Tub fan | Red |
| `GPIO 18` | IN3 | Exhaust fan | Red |
| `GPIO 19` | IN4 | Intake fan | Red |
| `GPIO 40` | IN5 | UVC lights | Red |
| `GPIO 41` | IN6 | Grow lights | Red |
| `GPIO 42` | IN7 | Pump (optional) | Red |
| `GPIO 47` | IN8 | Spare | Red |

All relay signals are the same type (active-LOW logic signals) — a single wire
colour is fine. Label each wire at both ends instead of relying on colour.

> **[?] Active LOW:** The relay fires (turns ON) when the IN pin is pulled to GND
> (LOW). It is OFF when the IN pin is HIGH (3.3V). This is the standard for PC817-
> based optocoupler relay boards. The ESP32 firmware uses `digitalWrite(pin, LOW)`
> to fire a relay and `digitalWrite(pin, HIGH)` to release it.

---

## Step 1 — Measure and Cut Wires

1. Route a piece of wire from the ESP32's GPIO header to the relay board's IN1
   pin without connecting it. Measure the length needed. Add 5 cm for strain relief.
2. Cut 8 wires at this length (or measure each individually if the routing paths
   differ significantly).
3. Strip ~5 mm of insulation from each end.
4. Crimp ferrules onto each stripped end.

---

## Step 2 — Label Every Wire

Before soldering or inserting anything, label each wire at both ends:

- One end: `GPIO 38` (or the GPIO number)
- Other end: `IN1` (or the relay IN number)

A label wrapped around the wire near the connection point is more durable than
a flag. Use heat-shrink labels if available.

Labelled wires are how you troubleshoot confidently in 18 months when you've
forgotten the exact wiring.

---

## Step 3 — Connect ESP32 GPIOs to Relay IN Pins

Work through the 8 channels one at a time:

1. **GPIO 38 → IN1 (Fogger):**
   - Insert one end into the ESP32 header at GPIO 38 position. If using a
     solderless connection, a female-to-female Dupont wire works here.
   - Insert/solder the other end to the relay board's IN1 pin.

2. **GPIO 39 → IN2 (Tub fan)**
3. **GPIO 18 → IN3 (Exhaust fan)**
4. **GPIO 19 → IN4 (Intake fan)**
5. **GPIO 40 → IN5 (UVC lights)**
6. **GPIO 41 → IN6 (Grow lights)**
7. **GPIO 42 → IN7 (Pump — optional; connect now for future use)**
8. **GPIO 47 → IN8 (Spare — optional; connect if you want to test all channels)**

> **Wire routing:** Route signal wires through the LV zone only — they should never
> cross the mains zone or run parallel to mains wiring for any significant length.
> Use cable ties to bundle the 8 relay signal wires together into a neat harness.

---

## Step 4 — Connect Relay Board VCC and GND

| Relay board pin | Connect to |
|----------------|-----------|
| VCC (optocoupler logic supply) | ESP32 `3V3` |
| JD-VCC (relay coil supply) | 5V PSU positive output |
| GND | Common GND |

**Important:** VCC and JD-VCC are now two separate connections (you removed the
jumper in Chapter 04). They must connect to different supplies:
- VCC → ESP32 3V3 (keeps the optocoupler logic at 3.3V)
- JD-VCC → 5V PSU (powers the relay coils)

> ⚠️ **SAFETY:** Do not connect JD-VCC to 3.3V. The relay coils are rated for
> 5V. Running them at 3.3V will result in weak magnetic actuation — the relay may
> click but not fully engage under load, causing intermittent contact issues.

---

## Step 5 — Verify Continuity Before Power-On

With all signal wires connected:

1. Set multimeter to continuity/resistance mode.
2. For each of the 8 channels, verify:
   - **Continuity between the ESP32 GPIO pin and the relay IN pin:** should show
     a path (low resistance through the wire).
   - **No continuity between any relay IN pin and the adjacent IN pin:** should
     show open circuit (no accidental short between channels).
3. Also verify:
   - VCC pin on relay board has continuity to ESP32 3V3
   - GND pin on relay board has continuity to ESP32 GND

**✓ Check:** All 8 GPIO-to-IN paths show continuity. No cross-channel shorts.
VCC and GND confirmed connected.

---

## Step 6 — Verify Failsafe Integration

Now that ESP32 GPIOs are also connected to the IN pins, re-run the Chapter 07
verification tests to confirm the combined wiring works:

1. Disconnect the ESP32 USB (so its GPIO pins are floating/held by pull-ups).
2. Set DPDT to MANUAL.
3. Close HUMIDITY switch.
4. Multimeter: IN1 to GND should show continuity (manual switch wins over floating GPIO).
5. Open HUMIDITY switch: IN1 returns to no-continuity (pull-up holds it HIGH).

**✓ Check:** Failsafe switches still work correctly with ESP32 GPIO wires also
attached to IN pins.

---

## Chapter 08 Checkpoint

- [ ] All 8 relay signal wires cut to correct length, labelled at both ends
- [ ] GPIO 38 → IN1 (Fogger)
- [ ] GPIO 39 → IN2 (Tub fan)
- [ ] GPIO 18 → IN3 (Exhaust fan)
- [ ] GPIO 19 → IN4 (Intake fan)
- [ ] GPIO 40 → IN5 (UVC lights)
- [ ] GPIO 41 → IN6 (Grow lights)
- [ ] GPIO 42 → IN7 (Pump — future use)
- [ ] GPIO 47 → IN8 (Spare)
- [ ] Relay board VCC → ESP32 3V3
- [ ] Relay board JD-VCC → 5V PSU output
- [ ] Relay board GND → common GND
- [ ] Continuity verified: all 8 GPIO-to-IN paths pass
- [ ] No cross-channel shorts between IN pins
- [ ] Failsafe re-verification passed
