# Chapter 04 — Relay Module Setup

**What you'll do:** Understand the relay board anatomy, remove the VCC/JD-VCC
jumper, verify the board is 3.3V compatible, install pull-up resistors on all 8
IN pins, and mount the board in the enclosure.

**Prerequisites:** Chapter 02 complete (standoffs installed). Relay module,
10 kΩ resistors ×8, soldering iron, solder, multimeter.

> ⚠️ **SAFETY:** No mains voltage. Bench work only. Do not power the relay board
> during this chapter.

---

## What Is a Relay?

<details>
<summary><strong>[?] Relay:</strong></summary>

A relay is an electrically controlled switch. It has two separate
circuits: a low-voltage "coil" circuit and a high-voltage "contact" circuit.
When a small signal pulls an IN pin LOW, the coil generates a magnetic field that
physically moves a metal arm to close the contact — you'll hear a click. The
contact switches the mains load without any electrical connection between the ESP32
signal and the mains voltage.

</details>

<details>
<summary><strong>[?] Optocoupler:</strong></summary>

An isolation layer inside the relay board. The ESP32's GPIO
signal drives a small LED inside the optocoupler. Light from that LED activates a
phototransistor on the other side, which drives the relay coil. The ESP32 and the
relay coil never share any electrical path — they communicate via light. This
protects the ESP32 from voltage spikes when inductive loads (fans) switch off.

</details>

---

## Relay Board Anatomy

A standard 8-channel opto-isolated relay board has:

```
┌───────────────────────────────────────────────────────────────┐
│                                                               │
│  Signal header:                 Relay indicators:            │
│  VCC  GND  IN1 IN2 ... IN8      8× LEDs (one per channel)    │
│  │    │    │   │       │                                      │
│  3-pin VCC/JD-VCC jumper        8× Relay bodies              │
│  ↓                                                            │
│  [VCC]──[JUMP]──[JD-VCC]        Load terminals (×8):         │
│                                  COM  NO  NC                  │
│                                  COM  NO  NC                  │
│                                  ...                          │
└───────────────────────────────────────────────────────────────┘
```

**Signal header** (left side) — faces the low-voltage zone:
- `VCC` — optocoupler power (3.3V from ESP32)
- `GND` — ground
- `IN1`–`IN8` — trigger inputs (active LOW)

**VCC/JD-VCC jumper** — you will remove this.

**Load terminals** (right side) — faces the mains zone:
- `COM` — common (mains Hot connects here)
- `NO` — normally open (closes when relay fires)
- `NC` — normally closed (not used in this build)

---

## Step 1 — Locate and Remove the VCC/JD-VCC Jumper

1. Find the 3-pin header near the signal connector, usually labelled
   "VCC–JD-VCC", bridged by a small plastic shorting cap.
2. Grip the jumper with needle-nose pliers and pull it straight off.
3. Store or discard it.

<details>
<summary><strong>[?] Why remove the jumper?</strong></summary>

With the jumper installed, the optocoupler logic
supply and the relay coil supply share the same rail. If a coil generates a voltage
spike on de-energise, that spike can couple back into the optocoupler side —
potentially reaching the ESP32. With the jumper removed:
- `VCC` (optocoupler side) connects to ESP32 3.3V
- `JD-VCC` (relay coil side) connects to 5V from the PSU

These two rails are electrically separate — coil spikes stay on the coil side.

</details>

**✓ Check:** Jumper removed. Multimeter continuity between VCC and JD-VCC shows
open circuit.

---

## Step 2 — Verify 3.3V Logic Compatibility

The relay module must respond reliably to 3.3V logic from the ESP32-S3.

**Measure the IN pin series resistors:**
1. Set multimeter to resistance mode.
2. Place one probe on IN1, other probe on GND.
3. Read the resistance. This is the optocoupler LED's series resistor.
   - **≤ 470 Ω** → ready for 3.3V logic ✓
   - **~1 kΩ** → the module is spec'd for 5V logic

**If the resistors are ~1 kΩ:**
Replace each IN pin's 1 kΩ resistor with 470 Ω. These are SMD components on the
relay board PCB — small but accessible with a fine-tip iron.

<details>
<summary><strong>[?] Why does the resistor value matter?</strong></summary>

At 3.3V logic with a 1 kΩ IN pin
resistor, the optocoupler LED sees ~2.1 mA. The PC817 optocoupler needs ~5 mA
for reliable activation. At 2.1 mA it may work at room temperature but drop out
as the board warms during a long fruiting cycle. At 470 Ω, drive current is ~4.5 mA
— within the reliable range regardless of temperature.

</details>

**✓ Check:** IN pin resistors confirmed ≤ 470 Ω on all 8 channels.

---

## Step 3 — Install Pull-Up Resistors on All 8 IN Pins

One 10 kΩ resistor per channel — 8 resistors total. Each connects from an IN pin
to the VCC rail (3.3V).

<details>
<summary><strong>[?] Why pull-up resistors?</strong></summary>

The relay fires when an IN pin is pulled LOW.
Before the ESP32 initialises its GPIO outputs at boot, GPIO pins are in an
undefined state and some can float LOW momentarily — which would fire relays at
every power-on. The 10 kΩ resistors hold every IN pin HIGH (relay OFF) from the
moment power is applied, before any firmware runs.

</details>

### Method A — Stripboard Adapter (Recommended)

Cut a small piece of stripboard. Solder resistors to bridge each IN pin row to a
VCC bus rail:

```
VCC bus rail ───────────────────────────────────────────
                    │       │       │       │   (×8)
                   10k     10k     10k     10k
                    │       │       │       │
               IN1 row  IN2 row  IN3 row  IN4 row  ...
```

Connect the adapter's IN rows to the relay board IN1–IN8 pins with short jumpers,
and its VCC row to the relay board VCC pin.

### Method B — Direct Solder to Header

Solder each 10 kΩ resistor directly between the VCC pin and an IN pin on the signal
header. Trim leads to ~5 mm before soldering to keep them neat.

**Verification after installation:**
1. Resistance mode — VCC to IN1 through IN8: each should read ~8–10 kΩ (the pull-up
   resistor in parallel with the optocoupler input impedance).
2. Between any two adjacent IN pins: should show open circuit (no solder bridge).

**✓ Check:** All 8 pull-up resistors installed. VCC→INx reads ~10 kΩ per channel.
No bridges between IN pins.

---

## Step 4 — Mount the Relay Board

1. Orient the relay board so:
   - **Load terminals (COM/NO/NC) face toward the mains zone**
   - **Signal header (IN1–IN8/VCC/GND) faces the LV zone**
2. Set the board on its M3 standoffs.
3. Secure with M3 screws — firm but not cracking-the-board tight.

**✓ Check:** Relay board mounted securely. Load terminals face mains zone. Signal
header faces LV zone. Board does not touch the enclosure floor.

---

## Chapter 04 Checkpoint

- [ ] VCC/JD-VCC jumper removed; VCC and JD-VCC confirmed open
- [ ] IN pin resistors confirmed ≤ 470 Ω on all channels
- [ ] 10 kΩ pull-up resistors installed on all 8 IN pins
- [ ] Pull-up resistors verified: VCC→INx reads ~10 kΩ per channel
- [ ] No solder bridges between IN pins
- [ ] Relay board mounted on standoffs; load terminals face mains zone

---

[← Ch 03 — Power Supply Installation](03-power-supply-install.md)  ·  [Ch 05 — ESP32-S3 Setup →](05-esp32-s3-setup.md)
