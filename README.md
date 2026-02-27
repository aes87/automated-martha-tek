# Automated Martha Tent — Build Guides

Everything needed to build and run an automated Martha tent mushroom fruiting chamber — from the initial hardware build through strain selection and growing. Based on u/dccrens' proven Martha 2.0 build; optional parts add a DIY ESP32-S3 controller, HA monitoring, and full grow tracking.

## How the Parts Fit Together

The tent build is self-contained and standalone — you can stop at Part 1 and have a working, automated fruiting chamber. Each part after that is an optional extension:

```
Martha tent (Part 1) ──────────────────────── complete standalone build (~$250)
  └── DIY ESP32-S3 controller + firmware (Parts 2A–2B) ── optional upgrade
      └── HA monitoring + Grafana (Part 3) ─────────────── optional, adds history

Once the tent is running:
  Inoculating (Part 4) ── strain selection, substrate prep, inoculation
    └── Growing (Part 5) ── fruiting, yield tracking, harvest
```

Parts 2A, 2B, and 3 replace or extend the off-the-shelf controllers — the physical tent, fogger, fans, and ducting stay the same.

## Contents

- [Part 1 — Martha Tent Build](#part-1--martha-tent-build)
- [Part 2 — Build a Controller *(optional)*](#part-2--build-a-controller-optional)
  - [Part 2A — DIY ESP32-S3 Controller](#part-2a--diy-esp32-s3-controller)
  - [Part 2B — Firmware](#part-2b--firmware)
- [Part 3 — Monitoring & Dashboard *(optional)*](#part-3--monitoring--dashboard-optional)
  - [Part 3A — HA Integration (via ha-tools)](#part-3a--ha-integration-via-ha-tools)
  - [Part 3B — Grafana + InfluxDB *(coming soon)*](#part-3b--grafana--influxdb-coming-soon)
- [Part 4 — Inoculating](#part-4--inoculating)
  - [Part 4A — Select / Buy Strains](#part-4a--select--buy-strains)
  - [Part 4B — Setup *(coming soon)*](#part-4b--setup-coming-soon)
- [Part 5 — Growing *(coming soon)*](#part-5--growing-coming-soon)
- [Credits](#credits)

---

## Part 1 — Martha Tent Build

A proven, fully automated Martha tent fruiting chamber — CO2-controlled FAE, humidity-controlled fogger, modular for easy cleaning. Based on u/dccrens' Martha 2.0 build shared on r/MushroomGrowers in January 2022 (274 upvotes).

**→ [Read the Martha Tent Build Guide](martha-tent-build/martha-tent-build-guide.md)**
**→ [Martha Tent Shopping List](https://aes87.github.io/automated-martha-tek/martha-tent-build/martha-tent-shopping-list.html)**

**What you build:** A greenhouse shelving tent with automated CO2 ventilation, external fogger reservoir, cross-flow airflow, and a robust anti-mold water treatment system.

**Cost: ~$250 USD** using off-the-shelf controllers.

This guide is a faithful writeup of dccrens' original build — a real, working system that has been running for over a year. **It stands completely on its own.** Parts 2, 3, and 4 below are optional extensions.

---

## Part 2 — Build a Controller *(optional)*

> **⚠️ Work in progress — not yet built**
>
> Parts 2A and 2B are an experiment in AI-assisted hardware and firmware design. The controller guide and firmware were written collaboratively with an AI assistant and have not been physically implemented or tested. If you want to build the DIY controller, either review the hardware, wiring, and firmware carefully yourself before trusting them, or wait for the human author to report back on how it goes.

The off-the-shelf CO2 and humidity controllers in the Martha tent build work well and are the safe, proven choice. Parts 2A and 2B describe a DIY ESP32-S3 board that replaces them — same tent, same fogger setup, just a different controller. The motivation is more sensors, per-shelf data, and a web dashboard rather than cost savings.

---

### Part 2A — DIY ESP32-S3 Controller

Replace the off-the-shelf CO2 and humidity controllers with a single ESP32-S3-based board. Hardware-adapted from u/mettalmag's open-source greenhouse controller (r/MushroomGrowers, Jan 2025).

**→ [Read the DIY Controller Build Guide](controller-build/hardware/diy-controller-build-guide.md)**
**→ [DIY Controller Shopping List](https://aes87.github.io/automated-martha-tek/controller-build/hardware/diy-controller-shopping-list.html)**
**→ [Step-by-Step Construction Guide](controller-build/hardware/construction-guide/chapters/00-overview.md)**

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

### Part 2B — Firmware

Open-source ESP32-S3 firmware for the DIY controller. PlatformIO + Arduino framework. Web dashboard at `http://martha.local`, REST API, OTA updates.

**→ [Firmware — README and quickstart](controller-build/firmware/README.md)**

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

---

## Part 3 — Monitoring & Dashboard *(optional)*

Connect the ESP32 controller to Home Assistant for persistent time-series logging, dashboards, and alerts. The controller's built-in web dashboard shows a live 30-minute window only — HA stores the full history and survives browser refreshes and reboots.

### Part 3A — HA Integration (via ha-tools)

> **⚠️ Work in progress — not yet tested against live hardware**

**What you get:**
- 16 sensors logged to HA history: CO2, RH (aggregate + per shelf), shelf temperatures, substrate temperatures ×5, water level, uptime
- 9 binary sensors: relay states (fogger, fans, UVC, lights, pump), controller armed, manual mode
- 4 automations: low water alert, high CO2 alert, low humidity alert, controller offline alert

Setup lives in the [ha-tools](../ha-tools/) project — run `node commands/martha-setup.js --help` to get started.

---

### Part 3B — Grafana + InfluxDB *(coming soon)*

An overkill but beautiful alternative: InfluxDB for long-term time-series storage + Grafana for dashboards with per-shelf overlays, VPD heatmaps, and substrate temperature trends.

This section is a placeholder — implementation coming once the hardware is running.

---

## Part 4 — Inoculating

Once the tent is built and running, the next question is what to actually grow in it.

### Part 4A — Select / Buy Strains

**→ [Mushroom Strain Guide](https://aes87.github.io/automated-martha-tek/growing/strain-guide.html)**

A reference covering 25 species and strains across three difficulty tiers — beginner, intermediate, and advanced. Each entry includes grow time, temperature range, substrate, Martha tent compatibility, flavor notes, fun facts, and buy links to major spawn vendors.

The guide is filterable by difficulty level and searchable by keyword.

---

### Part 4B — Setup *(coming soon)*

Substrate preparation, sterilization, and inoculation workflows for each strain. Coming once the first grows are underway.

---

## Part 5 — Growing *(coming soon)*

Everything once the colony is started: fruiting, environmental logs, per-batch yield tracking, harvest notes, and lessons learned.

---

## Credits

- **Martha tent build:** u/dccrens, r/MushroomGrowers — https://www.reddit.com/r/MushroomGrowers/comments/sbnlib/
- **DIY controller hardware reference:** u/mettalmag, r/MushroomGrowers — https://www.reddit.com/r/MushroomGrowers/comments/1rao1ms/
