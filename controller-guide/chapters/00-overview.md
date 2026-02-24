# Chapter 00 — Overview & How to Use This Guide

---

## What You're Building

You're building a custom Martha tent controller: a sealed electronics enclosure
that monitors conditions inside a fruiting chamber and automatically controls the
equipment that keeps your mushrooms happy.

The finished controller:

- **Reads** CO2, temperature, relative humidity (on three shelf heights),
  substrate temperature (five probes), light spectrum, and reservoir water level
- **Controls** eight loads via relay: fogger, tub fan, exhaust fan, intake fan,
  UVC reservoir lights, grow lights, and an optional top-off pump
- **Calculates** VPD (vapour pressure deficit) per shelf from the temp/RH data
- **Streams** everything to a web dashboard on your local network in real time
- **Fails safe**: physical panel switches let you run any load group manually,
  independent of the microcontroller

---

## What It Replaces

| Off-the-shelf controller | Price |
|--------------------------|-------|
| CO2 controller (dccrens) | ~$161 |
| Inkbird IHC200 humidity controller | ~$42 |
| Inkbird IBS-TH2 Plus monitor | ~$30 |
| **Total** | **~$233** |

The DIY controller costs **~$310–340** (base build, including GFCI protection and
per-load fusing that the off-the-shelf products don't include). You spend slightly
more — and you get dramatically more: per-shelf gradients, substrate temperature,
spectral light data, continuous water level, a web dashboard, and a system you
understand and can repair.

---

## How This Guide Is Structured

Thirteen chapters take you from an empty workbench to a running, flashed controller:

| Chapter | What happens |
|---------|-------------|
| 00 — This one | Overview, conventions, safety philosophy |
| 01 — Parts & Tools | Order everything; verify on arrival |
| 02 — Enclosure Prep | Mark zones, drill, mount DIN rail |
| 03 — Power Supply Install | Mount PSUs, GFCI, fuse holders (mechanical only) |
| 04 — Relay Module Setup | Remove jumper, add pull-up resistors, mount relay board |
| 05 — ESP32-S3 Setup | Identify pinout, bench-test, mount |
| 06 — Sensor Wiring | Wire every sensor to the ESP32 |
| 07 — Failsafe Panel | Drill switch holes, wire the manual override panel |
| 08 — Signal Wiring | Connect ESP32 GPIOs to relay inputs |
| 09 — Mains Wiring | Connect line voltage (GFCI, fuses, PSUs, loads) |
| 10 — Final Assembly | Checklists, photography, seal the enclosure |
| 11 — Power-On Test | Five structured tests before any loads are connected |
| 12 — Firmware Flash | Install tools, flash firmware, verify sensor readings |

Work in order. Chapters 02–08 are all **low-voltage bench work** — safe to do with
the enclosure unplugged. Chapter 09 is the only mains chapter. Chapter 11 requires
power but explicitly disconnects all load cables first.

---

## Icons and Conventions Used in This Guide

Throughout all chapters you will see:

> **[?] Term:** explanation of a concept — these are inline explainers. If you
> already know the term, skip them. In the future HTML version of this guide they
> will collapse.

> ⚠️ **SAFETY:** red-flag items. Read every one before proceeding. Never skip them
> in the mains wiring chapters.

**✓ Check:** a verification step — something you should confirm before moving on.
These are tests, not optional suggestions.

```
Code blocks
```
are used for wiring diagrams and terminal commands.

`GPIO 38`, `0x70`, `VCC` — backtick-formatted inline items are pin names, I2C
addresses, or signal labels.

- [ ] Checkbox items are checklists. Complete every item in a checklist before
  moving to the next chapter.

> **[DevKit V1]** callouts appear at the end of affected sections for anyone
> adapting this guide to the original ESP32 DevKit V1 board. They are secondary —
> this guide is written for the S3.

---

## Tools You Will Need

You need these before you can start Chapter 02:

| Tool | Why |
|------|-----|
| **Multimeter** | The most important tool. Used in nearly every chapter to verify connections before power-on. |
| **Soldering iron + solder** | Installing pull-up resistors in Ch 04; sensor wiring joints in Ch 06 |
| **Wire strippers** | Two sizes: 22 AWG (low-voltage), 14 AWG (mains) |
| **Ferrule crimping tool** | Crimps end-sleeves onto wire ends before inserting into screw terminals. Required for mains wiring; good practice everywhere |
| **Drill + step bit** | Cutting round holes in the enclosure for cable glands and panel switches |
| **Screwdrivers** | Flat + Phillips, for DIN rail and terminal block screws |
| **Label maker** | Label every wire at both ends; every component before wiring |
| **Wire cutters / snips** | Trimming component leads and cable gland excess |
| **Needle-nose pliers** | Seating connectors, tightening cable gland nuts |

> **[?] Ferrule crimping tool:** A hand-press tool that squeezes a small metal
> sleeve (a "ferrule") onto the stripped end of a wire. The sleeve prevents
> individual wire strands from splaying and causing short circuits when inserted
> into a screw terminal. Required for mains connections; strongly recommended
> everywhere in this build.

---

## Safety Philosophy

This build switches **mains voltage** — 120 V AC in North America, 230 V in Europe.
That voltage can kill at currents well below what trips a normal circuit breaker.
The design addresses this with multiple layers:

1. **GFCI/RCD** — trips at 5–30 mA in < 40 ms. The only protection fast enough
   to prevent electrocution. It is the first device on the mains feed. It is not
   optional.
2. **Sealed enclosure** — IP65 rated, gasket sealed. Keeps moisture off the
   electronics and keeps hands off live terminals.
3. **Two-zone layout** — mains-voltage components stay left; low-voltage
   electronics stay right. The relay board is the only component that bridges both
   zones, and its coil side is physically on the low-voltage side.
4. **Ferrule-terminated connections** — no bare wire strands near mains terminals.
5. **Per-load fusing** — a fault in one load channel blows a 2–3 A fuse, not the
   enclosure's main wiring.

**The rule while working in this enclosure:**

> ⚠️ **SAFETY:** Always unplug the enclosure from mains before touching anything
> inside it. Touch does not mean "touch a live wire" — it means **open the lid**. If
> the lid is open, the enclosure must be unplugged. No exceptions.

This rule applies in every chapter, including low-voltage chapters — because once
you reach Chapter 09, the mains zone is live, and muscle memory from Chapters 02–08
must include "unplug first".

---

## A Note on the ESP32-S3

This guide targets the **ESP32-S3 DevKitC-1** (38-pin variant with USB-C). It
resolves two hardware problems present in the original ESP32 DevKit V1:

- The V1 has a USB-to-UART bridge chip (CH340) that can oxidise in a humid
  environment. The S3 has native USB built into the chip — no bridge to corrode.
- The V1 uses GPIO 16 and 17 for its UART2 peripheral. GPIO 17 glitches LOW at
  boot, which would fire a relay. The S3 uses completely different GPIO numbering
  with no relay-relevant UART overlap.

> **[DevKit V1]** If you have a DevKit V1 and want to adapt: the GPIO numbers in
> this guide will not match your board. See the GPIO table in `CLAUDE.md` for the
> V1→S3 mapping. Every pin marked with a V1 note in Chapter 08 will need to change.

---

**Ready? Gather your parts list (Chapter 01) and let's build.**
