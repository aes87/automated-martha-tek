# Chapter 07 — Failsafe Panel Switches

**What you'll do:** Understand the AUTO/MANUAL system, install the 5 panel switches
into the enclosure lid, wire the DPDT master and four SPST group switches.

**Prerequisites:** Chapter 04 complete (relay board with pull-up resistors mounted).
5 panel switches (1× DPDT, 4× SPST), 22 AWG wire, soldering iron or ferrule
crimper.

> ⚠️ **SAFETY:** All wiring in this chapter is low-voltage (3.3V and GND). No mains
> voltage. Keep the enclosure unplugged.

---

## Why a Failsafe Panel?

The ESP32 is a computer. Computers crash, lose WiFi, or get stuck in a bad state.
If the ESP32 is the only way to control loads, a software bug or power cycle could
leave your mushrooms without humidity or fresh air for hours.

The failsafe panel lets you run any load group manually — without the ESP32, without
WiFi, without anything except flipping a switch.

> **[?] AUTO vs MANUAL — what actually changes:**
>
> In **AUTO** mode (normal operation): The ESP32 drives the relay IN pins. The
> panel switches' common wire is connected to 3.3V (VCC). If you close a group
> switch, it can only push an IN pin toward 3.3V, which is already its resting HIGH
> state (relay OFF). The manual switches have no effect — the ESP32 is in control.
>
> In **MANUAL** mode: The DPDT master switch connects the panel switches' common
> wire to GND. The group switches now directly pull relay IN pins LOW, firing the
> corresponding relays. The ESP32's GPIO pins are still connected to the IN pins,
> but the 10 kΩ pull-up resistors hold them HIGH (relay OFF) as long as the ESP32
> is not actively driving them.
>
> The key insight: in MANUAL mode, you flip the master switch first, then use the
> group switches to control loads. When done, flip the master back to AUTO.

---

## The Wiring Topology

```
                              ┌──── AUTO throw: connects common wire → VCC (3.3V)
DPDT MASTER switch ───────────┤     (manual switches have no effect in AUTO)
                              └──── MANUAL throw: connects common wire → GND
                                    (manual switches can now pull relay IN pins LOW)

Common wire from DPDT MANUAL throw ───────────────────────────────────────────┐
                                                                              │
SPST HUMIDITY switch ── one terminal ← common wire (MANUAL)                  │
                     ── other terminal → IN1 (fogger) + IN2 (tub fan)         │
                                                                              │
SPST FAE switch      ── one terminal ← common wire (MANUAL)                  │
                     ── other terminal → IN3 (exhaust) + IN4 (intake)         │
                                                                              │
SPST UVC switch      ── one terminal ← common wire (MANUAL)                  │
                     ── other terminal → IN5 (UVC lights)                     │
                                                                              │
SPST LIGHTS switch   ── one terminal ← common wire (MANUAL)                  │
                     ── other terminal → IN6 (grow lights)                    │
                                                                              │
ESP32 GPIOs ──── also connected to IN1–IN8 (always, in both AUTO and MANUAL) ┘
```

**What keeps the ESP32 from fighting the manual switches in MANUAL mode?**
The 10 kΩ pull-up resistors from Chapter 04. When the ESP32's GPIO is not actively
driven LOW, the pull-up holds the IN pin HIGH (relay OFF). If you then close a
group switch (which connects the IN pin to GND via the DPDT common wire), the GND
wins — the 10 kΩ pull-up resistor limits current but allows the IN pin to be pulled
LOW by the manual switch. The firmware also includes a MANUAL mode input GPIO that
releases all relay outputs when detected, but the pull-ups alone provide safe
default behaviour.

---

## Step 1 — Install the DPDT Master Switch

1. The lid should have its leftmost switch hole labelled "MASTER AUTO/MAN" from
   Chapter 02.
2. Thread the DPDT switch body through the hole from outside.
3. Thread the mounting nut onto the bushing from inside and tighten firmly.
   Use needle-nose pliers to snug it — not so tight you crack the plastic, but
   firm enough the switch won't rotate.
4. Optionally fit the switch lever with an indicator label or coloured cap if your
   switches came with them.

> **[?] DPDT switch:** "Double Pole Double Throw" — it has 6 terminals. Two
> independent switching poles (A and B), each with 3 terminals: common (centre),
> throw 1 (up), throw 2 (down). When the lever is in one position, each pole's
> common connects to throw 1. In the other position, each pole's common connects
> to throw 2. It's like two SPST switches that move together.

---

## Step 2 — Install the Four SPST Group Switches

Install the 4 SPST switches in the remaining 4 holes, in order from left to right:
HUMIDITY, FAE, UVC, LIGHTS.

> **[?] SPST switch:** "Single Pole Single Throw" — the simplest switch type.
> Two terminals. When the lever is ON, the terminals are connected. When OFF,
> they're open. Acts as a gate on the common wire.

---

## Step 3 — Wire the DPDT Master Switch

The DPDT has 6 terminals. Identify them (refer to your specific switch's datasheet;
a common arrangement is two rows of 3 pins):

```
DPDT terminal layout (top view):
  [P1-COM]  [P1-T1]  [P1-T2]
  [P2-COM]  [P2-T1]  [P2-T2]

P1 = Pole A (the switching pole for the common wire)
P2 = Pole B (optional — can connect an LED indicator)
```

**Wire Pole A (the essential pole):**

| Terminal | Connect to | Wire colour (suggestion) |
|----------|-----------|--------------------------|
| P1-COM | Short wire to a junction node — this becomes the "common wire" that all SPST group switches connect to | Orange |
| P1-T1 (AUTO position) | ESP32 `3V3` | Red |
| P1-T2 (MANUAL position) | ESP32 GND (common GND) | Black |

Confirm which throw is AUTO and which is MANUAL by testing with a multimeter
(continuity mode) before soldering: with the lever up, check which pair of
terminals are connected.

**Wire Pole B (optional — LED indicator):**

If you want a panel LED to indicate MANUAL mode is active:
- Connect an LED + 1 kΩ series resistor between P2-T2 (MANUAL throw) and GND
- P2-COM connects to 3.3V
- In MANUAL position: P2-COM connects through the LED to GND → LED lights up
- In AUTO position: P2-COM connects to P2-T1 (which you leave unconnected) → LED off

If you skip the LED, leave Pole B unconnected.

**✓ Check:** Multimeter continuity mode — DPDT in AUTO: continuity between COM
and 3.3V side. DPDT in MANUAL: continuity between COM and GND side.

---

## Step 4 — Wire the SPST Group Switches

Each SPST group switch has 2 terminals. One terminal connects to the DPDT common
wire junction; the other connects to the relay IN pin(s) it controls.

Wire each switch:

| Switch | Terminal 1 | Terminal 2 |
|--------|-----------|-----------|
| HUMIDITY | DPDT common wire | Relay `IN1` (fogger) and `IN2` (tub fan) — both |
| FAE | DPDT common wire | Relay `IN3` (exhaust) and `IN4` (intake) — both |
| UVC | DPDT common wire | Relay `IN5` (UVC lights) |
| LIGHTS | DPDT common wire | Relay `IN6` (grow lights) |

For HUMIDITY and FAE, where one switch controls two relay IN pins:
- From the switch terminal, run a short wire to a small terminal block (or twist
  two wires together at a Wago connector) and then two separate wires to IN1 and IN2
  (or IN3 and IN4).

> ⚠️ **SAFETY:** These wires carry at most a few milliamps and are never connected
> to mains. They run between the panel switches (on the lid) and the relay board
> (inside the enclosure). Use 22 AWG stranded wire. Leave enough slack for the lid
> to open fully without pulling on the connections.

---

## Step 5 — Verify Failsafe Wiring (No Mains Yet)

Before any power is applied, verify the failsafe circuit with a multimeter.

**Test 1 — AUTO mode (DPDT in AUTO position):**
1. Set DPDT to AUTO.
2. Close HUMIDITY switch.
3. Multimeter continuity between relay IN1 and GND: **no continuity** (open).
4. Repeat for IN2, IN3, IN4, IN5, IN6: all should show no continuity to GND.

*What you should see:* All readings show open circuit. In AUTO position, closing
a group switch connects the relay IN pin to 3.3V, not GND — no relay fires.

**Test 2 — MANUAL mode (DPDT in MANUAL position):**
1. Set DPDT to MANUAL.
2. Keep all group switches open.
3. Multimeter continuity between relay IN1 and GND: **no continuity** (pull-up
   holds IN pin HIGH).
4. Close HUMIDITY switch.
5. Multimeter between relay IN1 and GND: **continuity** (IN pin pulled to GND
   via closed switch).
6. Between relay IN2 and GND: **continuity** (HUMIDITY switch also pulls IN2).
7. Open HUMIDITY switch: IN1 and IN2 return to no-continuity.
8. Repeat for FAE (IN3+IN4), UVC (IN5), LIGHTS (IN6).

*What you should see:* In MANUAL mode, each closed group switch shows continuity
to GND on its corresponding IN pins. Opening the switch breaks continuity.

**✓ Check:** All 8 tests pass. MANUAL mode enables each group switch correctly.
AUTO mode makes group switches inoperative.

---

## Chapter 07 Checkpoint

- [ ] All 5 switches installed in lid with mounting nuts secure
- [ ] DPDT master switch wired: COM → common wire node; T1 (AUTO) → 3.3V; T2 (MANUAL) → GND
- [ ] HUMIDITY switch: common wire → IN1 and IN2
- [ ] FAE switch: common wire → IN3 and IN4
- [ ] UVC switch: common wire → IN5
- [ ] LIGHTS switch: common wire → IN6
- [ ] Sufficient wire slack for lid to open fully
- [ ] Failsafe verification Test 1 passed (AUTO mode: group switches inactive)
- [ ] Failsafe verification Test 2 passed (MANUAL mode: each switch activates correct IN pins)
