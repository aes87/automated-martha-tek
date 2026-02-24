# Chapter 04 — Relay Module Setup

**What you'll do:** Understand the relay board anatomy, remove the VCC/JD-VCC
jumper, verify 3.3V compatibility, install pull-up resistors on all 8 IN pins,
and mount the board in the enclosure.

**Prerequisites:** Chapter 02 complete (standoffs installed). Relay board, 10 kΩ
resistors ×8, soldering iron, solder, multimeter, small screwdriver.

> ⚠️ **SAFETY:** No mains voltage. This is bench work on a low-voltage component.
> The relay board should not be powered during this chapter.

---

## What Is a Relay?

> **[?] Relay:** A relay is an electrically controlled switch. It has two separate
> circuits: a low-voltage "coil" circuit and a high-voltage "contact" circuit.
> When you apply a small signal to the coil (here, pulling an IN pin LOW from
> the ESP32), the coil generates a magnetic field that physically moves a metal
> arm to close the contact — you'll hear a click. The contact switches the mains
> load (fan, fogger, etc.) without any electrical connection between the ESP32
> signal and the mains voltage. This isolation is what makes it safe.

> **[?] Optocoupler:** An additional isolation layer inside the relay board. The
> ESP32's GPIO signal goes into a small LED inside the optocoupler. Light from that
> LED activates a phototransistor on the other side, which in turn drives the relay
> coil. The ESP32 and the relay coil never share any electrical path — they
> communicate via light. This protects the ESP32 from voltage spikes when inductive
> loads (fans) switch off and create brief back-EMF pulses.

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

**Signal header** (left side): This faces the low-voltage zone in the enclosure.
- `VCC` — optocoupler power (will connect to ESP32 3.3V)
- `GND` — ground
- `IN1` through `IN8` — relay trigger inputs (active LOW — relay fires when pulled
  to GND)

**VCC/JD-VCC jumper** (near the signal header): A small plastic shorting bridge.
When present, it connects the optocoupler logic supply and the relay coil supply
together. You will remove this.

**Load terminals** (right side): This faces the mains zone. Each relay channel has
three terminals:
- `COM` — common (mains Hot connects here)
- `NO` — normally open (open when relay is off; closed when relay fires)
- `NC` — normally closed (closed when relay is off; open when relay fires)

For this build you use `COM` and `NO` on every channel. When the relay fires, it
connects `COM` to `NO`, completing the circuit to the load.

---

## Step 1 — Locate and Remove the VCC/JD-VCC Jumper

1. Find the 3-pin header near the signal connector. It is usually labelled
   "VCC–JD-VCC" or similar, with a shorting jumper (a small plastic cap with a
   metal insert) bridging two of the pins.
2. Grip the jumper with needle-nose pliers or tweezers and pull it straight off.
3. Store the jumper somewhere safe (you might want it for other projects) or
   discard it.

> **[?] Why remove the jumper?** When the jumper is installed, `VCC` and `JD-VCC`
> are the same supply rail. This means the relay coils and the optocoupler logic
> both share the same power source. If a relay coil generates a voltage spike when
> it de-energises, that spike can couple back through the shared rail into the
> optocoupler side — and potentially into the ESP32.
>
> With the jumper removed:
> - `VCC` (optocoupler side) connects to ESP32 3.3V — the logic side
> - `JD-VCC` (relay coil side) connects to 5V from the PSU — the power side
>
> These two rails are now electrically separate. Coil spikes stay on the coil
> side and never reach the ESP32.

**✓ Check:** Jumper is removed. The VCC and JD-VCC pins are not bridged (verify
with multimeter: continuity between VCC and JD-VCC should show open circuit after
jumper removal).

---

## Step 2 — Verify 3.3V Compatibility

You should have checked this in Chapter 01. Confirm now before soldering:

**Method A — Module listing:** If the listing explicitly says "3.3V trigger" or
"3.3V compatible", you're good.

**Method B — Resistor measurement:**
1. Set multimeter to resistance mode.
2. Place one probe on the IN1 pin of the signal header.
3. Place the other probe on the GND pin.
4. Read the resistance. You are reading the optocoupler LED's series resistor.
   - Reading ~470 Ω (or less): Module is ready for 3.3V logic. ✓
   - Reading ~1000 Ω (~1 kΩ): Module uses 5V logic resistors. See fix below.

**If the resistors are ~1 kΩ (5V logic):**
Option 1: Source a 3.3V-compatible relay module and use that instead.
Option 2: Replace each IN pin's 1 kΩ resistor with a 470 Ω resistor. These are
tiny SMD components on the relay board PCB. This requires steady hands and a
soldering iron with a fine tip. It is doable but not trivial.

**✓ Check:** IN pin resistors confirmed ≤ 470 Ω or module is confirmed 3.3V
compatible by specification.

---

## Step 3 — Install Pull-Up Resistors on All 8 IN Pins

> **[?] Why pull-up resistors on relay inputs?** The relay module triggers when an
> IN pin is pulled LOW. Before the ESP32 initialises its GPIO outputs at boot, all
> GPIOs are in an undefined state — some can float LOW momentarily. Without
> pull-ups, this would fire relays at every power-on. The 10 kΩ resistors hold
> every IN pin HIGH (relay OFF) from the moment power is applied, before any
> firmware runs. The ESP32 then actively drives pins LOW to fire relays when it
> wants to.

You will install one 10 kΩ resistor per channel — 8 resistors total. Each one
connects from an IN pin to the VCC rail (the optocoupler logic supply, 3.3V).

### Method A — Stripboard Adapter (Recommended)

Cut a small piece of stripboard (also called "veroboard") with 8 rows. Solder
the resistors across the strips to connect each IN pin row to a VCC rail row:

```
VCC rail (bus strip) ──────────────────────────────
                        │     │     │     │   ×8
                       10k   10k   10k   10k  ...
                        │     │     │     │
IN1 row         IN2 row  IN3 row  IN4 row  ...
```

Then use short wire jumpers to connect this adapter board's IN rows to the relay
board's IN1–IN8 pins, and its VCC row to the relay board's VCC pin.

### Method B — Direct Solder to Relay Board Header

If you are comfortable soldering:

1. Cut 8 pieces of 10 kΩ resistor with leads trimmed to ~5 mm.
2. Bridge each resistor between adjacent VCC and IN pin holes on the signal header.
   The VCC pin and the IN1–IN8 pins are in a row — the resistors bridge across.
3. Verify each resistor is soldered to both VCC and its corresponding IN pin —
   not bridging two adjacent IN pins.

**Verification after installation:**
1. Set multimeter to resistance mode.
2. Between VCC pin and IN1: should read approximately 10 kΩ (you are reading the
   pull-up resistor in parallel with the optocoupler input impedance — expect
   8–10 kΩ).
3. Repeat for IN2 through IN8.
4. Between any two adjacent IN pins: should **not** show a near-zero reading
   (that would indicate a solder bridge).

**✓ Check:** All 8 pull-up resistors installed. Resistance VCC→INx reads 8–10 kΩ
on each channel. No bridges between IN pins.

---

## Step 4 — Mount the Relay Board in the Enclosure

1. Orient the relay board so that:
   - **Load terminals (COM/NO/NC) face toward the mains zone** (left)
   - **Signal header (VCC/GND/IN1–IN8) faces the LV zone** (right)
2. Set the board on the M3 standoffs you installed in Chapter 02.
3. Secure with M3 screws — finger-tight first, then firm.

The relay board physically spans the zone divider. Its load contacts are in the
mains zone; its signal header is in the LV zone. This is the correct and safe
orientation.

**✓ Check:** Relay board is mounted securely. Load terminals are oriented toward
the mains zone. Signal header is oriented toward the LV zone. No metal parts of
the board touch the enclosure floor (standoffs provide clearance).

---

## Chapter 04 Checkpoint

- [ ] VCC/JD-VCC jumper removed; VCC and JD-VCC confirmed open with multimeter
- [ ] IN pin resistors confirmed ≤ 470 Ω (or module confirmed 3.3V compatible)
- [ ] 10 kΩ pull-up resistors installed on all 8 IN pins
- [ ] Pull-up resistors verified: VCC→INx reads ~10 kΩ for each channel
- [ ] No solder bridges between IN pins
- [ ] Relay board mounted on standoffs, load terminals facing mains zone
- [ ] Signal header facing LV zone
