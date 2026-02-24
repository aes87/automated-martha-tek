# Chapter 00 — Overview & How to Use This Guide

---

## What You're Building

You're assembling a Martha tent controller: a sealed electronics enclosure that
monitors conditions inside a fruiting chamber and automatically controls the
equipment that keeps your mushrooms healthy.

The finished controller:

- **Reads** CO2, temperature, relative humidity (at three shelf heights), substrate
  temperature (five probes), light spectrum, and reservoir water level
- **Controls** eight loads via relay: fogger, tub fan, exhaust fan, intake fan,
  UVC reservoir lights, grow lights, and an optional top-off pump
- **Calculates** VPD (vapour pressure deficit) per shelf from temp/RH data
- **Streams** everything to a web dashboard on your local network in real time
- **Fails safe**: physical panel switches let you run any load group manually,
  independent of the microcontroller

---

## How This Guide Is Structured

Thirteen chapters take you from an empty workbench to a running, flashed controller:

| Chapter | What happens |
|---------|-------------|
| [00 — This one](00-overview.md) | Overview, conventions, safety philosophy |
| [01 — Parts & Tools](01-parts-and-tools.md) | What to have in hand before starting |
| [02 — Enclosure Prep](02-enclosure-prep.md) | Mark zones, drill, mount DIN rail |
| [03 — Power Supply Install](03-power-supply-install.md) | Mount PSUs, GFCI, fuse holders (mechanical only) |
| [04 — Relay Module Setup](04-relay-module-setup.md) | Remove jumper, add pull-up resistors, mount relay board |
| [05 — ESP32-S3 Setup](05-esp32-s3-setup.md) | Identify pinout, bench-test, mount |
| [06 — Sensor Wiring](06-sensor-wiring.md) | Wire every sensor to the ESP32 |
| [07 — Failsafe Panel](07-failsafe-panel.md) | Drill switch holes, wire the manual override panel |
| [08 — Signal Wiring](08-signal-wiring.md) | Connect ESP32 GPIOs to relay inputs |
| [09 — Mains Wiring](09-mains-wiring.md) | Connect line voltage (GFCI, fuses, PSUs, loads) |
| [10 — Final Assembly](10-final-assembly.md) | Checklists, photography, seal the enclosure |
| [11 — Power-On Test](11-power-on-test.md) | Five structured tests before any loads are connected |
| [12 — Firmware Flash](12-firmware-flash.md) | Install tools, flash firmware, verify sensor readings |

Work in order. Chapters 02–08 are all **low-voltage bench work** — safe to do with
the enclosure unplugged. Chapter 09 is the only mains chapter. Chapter 11 requires
power but explicitly disconnects all load cables first.

---

## Icons and Conventions Used in This Guide

Throughout all chapters you will see:

<details>
<summary><strong>[?] Term</strong></summary>

Inline concept explainers — collapsible. If you already know the term, skip it.

</details>

Safety and information callouts use GitHub's native alert format:

> [!CAUTION]
> Mains voltage, electrocution risk, or component destruction. Never skip.

> [!WARNING]
> Physical or equipment damage risk. Read before proceeding.

> [!IMPORTANT]
> A configuration detail that must be correct for the build to work safely.

> [!NOTE]
> Confirms you're in a safe state, or provides useful context.

Superscript numbers like this[^example] link to brief **"learn more"** notes at
the bottom of the chapter — extra context for the curious. They are never required
reading for completing the build.

[^example]: Like this. Footnotes appear at the bottom of the page on GitHub.

**✓ Check:** a verification step — something you should confirm before moving on.
These are tests, not optional suggestions.

```
Code blocks
```
are used for wiring diagrams and terminal commands.

`GPIO 38`, `0x70`, `VCC` — backtick-formatted inline items are pin names, I2C
addresses, or signal labels.

- [ ] Checkbox items are checklists. Complete every item before moving to the next
  chapter.

---

## Tools You Will Need

| Tool | Why |
|------|-----|
| **Multimeter** | The most important tool. Used in nearly every chapter to verify connections before power-on. |
| **Soldering iron + solder** | Installing pull-up resistors in Ch 04; sensor wiring joints in Ch 06 |
| **Wire strippers** | Two sizes: 22 AWG (low-voltage), 14 AWG (mains) |
| **Ferrule crimping tool** | Crimps end-sleeves onto wire ends before inserting into screw terminals |
| **Drill + step bit** | Cutting round holes in the enclosure for cable glands and panel switches |
| **Screwdrivers** | Flat + Phillips, for DIN rail and terminal block screws |
| **Label maker** | Label every wire at both ends; every component before wiring |
| **Wire cutters / snips** | Trimming component leads and cable gland excess |
| **Needle-nose pliers** | Seating connectors, tightening cable gland nuts |

<details>
<summary><strong>[?] Ferrule crimping tool:</strong></summary>

A hand-press tool that squeezes a small metal
sleeve (a "ferrule") onto the stripped end of a wire. The sleeve prevents
individual wire strands from splaying and causing short circuits when inserted
into a screw terminal. Required for mains connections; strongly recommended
everywhere in this build.

</details>

---

## Safety Philosophy

This build switches **mains voltage** — 120 V AC in North America, 230 V in Europe.
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

> [!CAUTION]
> Always unplug the enclosure from mains before touching anything
> inside it. "Touching" means **opening the lid**. If the lid is open, the enclosure
> must be unplugged. No exceptions.

Build this habit during low-voltage chapters so it's automatic when mains is live.

---

[Ch 01 — Parts & Tools →](01-parts-and-tools.md)
