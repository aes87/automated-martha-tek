# Martha Tent Controller — Construction Guide

## Purpose

A step-by-step hobbyist build guide for the DIY Martha Tent Controller. Transforms
the technical source documents in `setup/docs/` into a format a first-time ESP32
builder can follow — with inline concept explainers, a novice-safe parts list, and
verification steps at every stage.

**Scope:** Hardware assembly + firmware flash only. Tent integration (sensor
placement, automation tuning) is covered by the broader setup documentation.

**Primary platform:** ESP32-S3 DevKitC-1.
**Water level path:** DFRobot KIT0139 only (built-in overvoltage protection).

---

## Chapter Index

| File | Title |
|------|-------|
| `chapters/00-overview.md` | Overview & How to Use This Guide |
| `chapters/01-parts-and-tools.md` | Parts & Tools |
| `chapters/02-enclosure-prep.md` | Enclosure Preparation |
| `chapters/03-power-supply-install.md` | Power Supply Installation (Mechanical) |
| `chapters/04-relay-module-setup.md` | Relay Module Setup |
| `chapters/05-esp32-s3-setup.md` | ESP32-S3 Setup |
| `chapters/06-sensor-wiring.md` | Sensor Wiring |
| `chapters/07-failsafe-panel.md` | Failsafe Panel Switches |
| `chapters/08-signal-wiring.md` | Low-Voltage Signal Wiring |
| `chapters/09-mains-wiring.md` | Mains Wiring |
| `chapters/10-final-assembly.md` | Final Assembly Checklist |
| `chapters/11-power-on-test.md` | Initial Power-On Test |
| `chapters/12-firmware-flash.md` | Firmware Flash |

---

## Writing Conventions

Every chapter follows these patterns:

| Element | Markdown pattern | Future HTML intent |
|---------|-----------------|-------------------|
| Inline explainer | `> **[?] Term:** explanation` blockquote | `<details><summary>` collapsible |
| Safety callout | `> ⚠️ **SAFETY:** ...` | Styled warning box |
| Verification step | `**✓ Check:** what you should see/hear/measure` | Green check box |
| Checkpoint list | `- [ ] task` | Interactive checkbox |
| GPIO / part callout | backtick `` `GPIO 38` `` | Code span |

DevKit V1 differences (where they differ from S3) appear in `> **[DevKit V1]**`
blockquotes at the end of affected sections — secondary, not primary.

---

## S3 GPIO Reference (from `hal.h`)

| GPIO | Function | Notes |
|------|----------|-------|
| 4 | 1-Wire (DS18B20 ×5) | 2.2 kΩ pull-up + 100 nF decoupling |
| 7 | ADC — Water level | ADC1_CH6; KIT0139 converter board used |
| 9 | I2C SCL | Default S3 Arduino framework |
| 18 | Relay Ch3 — Exhaust fan | Safe boot default |
| 19 | Relay Ch4 — Intake fan | Safe boot default |
| 21 | I2C SDA | Same as V1 |
| 38 | Relay Ch1 — Fogger | Clean GPIO; safe boot |
| 39 | Relay Ch2 — Tub fan | Clean GPIO; safe boot |
| 40 | Relay Ch5 — UVC lights | 10 s boot guard (firmware) |
| 41 | Relay Ch6 — Grow lights | Safe boot |
| 42 | Relay Ch7 — Pump (opt.) | Safe boot |
| 47 | Relay Ch8 — Spare | Safe boot |

---

## Source Material

| File | Role |
|------|------|
| `setup/docs/diy-controller-build-guide.md` | Primary technical source |
| `setup/docs/diy-controller-hardware-reference.md` | Sensor rationale, BOM, alternatives |
| `mushroom-firmware/include/hal.h` | S3 GPIO pin mapping |
| `mushroom-firmware/include/config.h` | Relay pins, boot timing, thresholds |

---

## When Working Here

1. Read this file and the source material before adding or modifying chapters
2. Cross-check every GPIO number against `hal.h` — do not use V1 numbers for S3 chapters
3. Water level wiring must reference KIT0139 path only; do not expand the DIY shunt path
4. Every mains section must include `> ⚠️ **SAFETY:**` callouts; never skip them
5. After writing a chapter, verify: steps have checkpoints, explainers use `[?]` format,
   S3 GPIO numbers are correct, mains sections have safety warnings
