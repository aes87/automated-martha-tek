# Automated Martha Tent — Build Guides

A writeup of u/dccrens' proven Martha tent build, plus an optional DIY ESP32-S3 controller and firmware that replaces the off-the-shelf controllers with something more data-rich.

## Part 1 — Martha Tent Build

A proven, fully automated Martha tent fruiting chamber — CO2-controlled FAE, humidity-controlled fogger, modular for easy cleaning. Based on u/dccrens' Martha 2.0 build shared on r/MushroomGrowers in January 2022 (274 upvotes).

**→ [Read the Martha Tent Build Guide](martha-tent-build-guide.md)**

**What you build:** A greenhouse shelving tent with automated CO2 ventilation, external fogger reservoir, cross-flow airflow, and a robust anti-mold water treatment system.

**Cost: ~$250 USD** using off-the-shelf controllers.

This guide is a faithful writeup of dccrens' original build — a real, working system that has been running for over a year. **It stands completely on its own.** Parts 2 and 3 below are optional extensions.

---

## Parts 2 & 3 — DIY Controller and Firmware (optional)

> **⚠️ Work in progress — not yet built**
>
> Parts 2 and 3 are an experiment in AI-assisted hardware and firmware design. The controller guide and firmware were written collaboratively with an AI assistant and have not been physically implemented or tested. If you want to build the DIY controller, either review the hardware, wiring, and firmware carefully yourself before trusting them, or wait for the human author to report back on how it goes.

The off-the-shelf CO2 and humidity controllers in the Martha tent build work well and are the safe, proven choice. Parts 2 and 3 describe a DIY ESP32-S3 board that replaces them — same tent, same fogger setup, just a different controller. The motivation is more sensors, per-shelf data, and a web dashboard rather than cost savings.

---

## Part 2 — DIY ESP32-S3 Controller *(optional)*

Replace the off-the-shelf CO2 and humidity controllers with a single ESP32-S3-based board. Hardware-adapted from u/mettalmag's open-source greenhouse controller (r/MushroomGrowers, Jan 2025).

**→ [Read the DIY Controller Build Guide](diy-controller-build-guide.md)**

**What you build:** An ESP32-S3 controller in an IP65 enclosure with:
- Sensirion SCD30 — CO2, temperature, humidity
- SHT45 ×3 via TCA9548A — per-shelf humidity and temperature
- DS18B20 ×5 — substrate temperature per shelf
- AS7341 — 11-channel spectral light monitoring
- Submersible pressure sensor — continuous reservoir water level
- 8-channel relay module — fogger, FAE fans, UVC, lights, optional pump
- Physical failsafe panel — manual override independent of the ESP32-S3

**Cost: ~$290–360 USD** — roughly the same as the off-the-shelf controllers it replaces, with dramatically more capability.

> **Optional add-on:** automatic reservoir top-off pump (pre-wired in base build, ~$20 to activate).

---

## Part 3 — Firmware *(optional, requires Part 2)*

Open-source ESP32-S3 firmware for the DIY controller. PlatformIO + Arduino framework. Web dashboard at `http://martha.local`, REST API, OTA updates.

**→ [Firmware — README and quickstart](firmware/README.md)**

**What it does:**
- Humidity control loop: SHT45 → fogger + tub fan (hysteresis 85–87% RH, 30s cooldown)
- CO2/FAE control loop: SCD30 → exhaust + intake fans (hysteresis 800–950 ppm, 60s min run)
- Timer scheduler: UVC duty cycle (1h on / 4h off) and lights daily window (08:00–20:00), NTP-synced
- Pump control: water level → reservoir top-off relay
- Web dashboard: live sensor cards, 30-minute sparklines, relay toggles, config panel
- REST API: read/write config, manual relay override, log level
- OTA updates: ElegantOTA at `/update`
- Hardware watchdog: 30s reset guard
- Config persistence: all settings survive power cycles via NVS

**Build and flash:**
```bash
cd firmware
pio run -e esp32s3 --target upload       # flash firmware
pio run -e esp32s3 --target uploadfs     # flash web UI (LittleFS)
pio test -e native                       # run unit tests on PC (no hardware needed)
```

---

## How the Parts Fit Together

The tent guide is self-contained — you can build it exactly as dccrens designed it, using the Inkbird humidity controller and CO2 controller. The DIY controller guide is a drop-in upgrade that replaces those two controllers (and the Inkbird IBS-TH2 Plus monitor) with the ESP32-S3 board. The firmware is the software for that board.

The physical tent, fogger system, fans, ducting, and everything else stays the same.

```
dccrens Martha 2.0 tent
  └── replace: CO2 controller + Inkbird IHC200 + Inkbird IBS-TH2 Plus
      └── with: DIY ESP32-S3 controller (Part 2) + this firmware (Part 3)
```

---

## Credits

- **Martha tent build:** u/dccrens, r/MushroomGrowers — https://www.reddit.com/r/MushroomGrowers/comments/sbnlib/
- **DIY controller hardware reference:** u/mettalmag, r/MushroomGrowers — https://www.reddit.com/r/MushroomGrowers/comments/1rao1ms/
