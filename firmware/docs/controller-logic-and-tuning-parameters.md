# Controller Logic and Tuning Parameters

This document explains how the Martha Tent Controller makes decisions — what it measures, what it actuates, and how the control loops work. It also covers every tunable parameter: what it does, its default value, and when (and when not) to change it.

---

## Architecture Overview

The controller runs four independent subsystems in parallel:

1. **Humidity loop** — closed-loop RH control via fogger
2. **CO₂ / FAE loop** — closed-loop CO₂ control via exhaust + intake fans
3. **Timer scheduler** — open-loop time-based control for grow lights and UVC
4. **Water level loop** — closed-loop reservoir management via pump

Each subsystem reads from the shared `SensorSnapshot` (populated every 2 seconds by a FreeRTOS sensor task) and writes relay commands via `RelayManager`. The control task runs at 1-second resolution.

---

## Humidity Loop

**Sensors:** 3× SHT45 (top / mid / low shelf via TCA9548A I2C mux)
**Actuators:** Relay Ch1 — Fogger, Relay Ch2 — Tub fan (always toggled together)

The three shelf sensors are aggregated into a single `rh_aggregate_pct` value before the control decision. The aggregation mode is configurable:

| Mode | Behaviour |
|------|-----------|
| `AVERAGE` (default) | Mean of all valid shelf readings |
| `MIN` | Lowest reading — fogger runs whenever *any* shelf is dry |
| `MAX` | Highest reading — fogger runs only when *all* shelves are wet |

The loop is a simple hysteresis controller:

```
if rh_aggregate < RH_ON_PCT  → turn fogger ON
if rh_aggregate > (RH_ON_PCT + RH_HYSTERESIS_PCT)  → turn fogger OFF
```

A **cooldown timer** (`HUMIDITY_COOLDOWN_MS`, 30 s) prevents the fogger from switching state again until the cooldown expires. This protects the relay and fogger motor from rapid cycling when the tent is near the setpoint.

### VPD — derived, not controlled

VPD is calculated from each shelf's temperature and humidity reading:

```
SVP  = 0.6108 × exp(17.27 × T / (T + 237.3))   kPa
VPD  = SVP × (1 − RH / 100)                      kPa
```

VPD is a **read-only diagnostic metric**. The controller has no direct actuator for temperature, so VPD cannot be closed-loop controlled. The humidity setpoint is the only knob that influences it; temperature is set by the room environment.

Target range for gourmet fruiting species: **0.3–0.8 kPa**. High VPD → cracking, aborted pins. Low VPD → bacterial blotch risk.

---

## CO₂ / FAE Loop

**Sensor:** SCD30 (dual-channel NDIR, mid-height back wall)
**Actuators:** Relay Ch3 — Exhaust fan, Relay Ch4 — Intake fan (always toggled together)

```
if co2_ppm > CO2_ON_PPM   → turn FAE fans ON
if co2_ppm < CO2_OFF_PPM  → turn FAE fans OFF
```

A **minimum run time** (`FAE_MIN_RUN_MS`, 60 s) forces the fans to stay on for at least one full air exchange once triggered. This prevents the fans from cutting off prematurely when CO₂ briefly dips below the off threshold as fresh air mixes in.

### Interaction with humidity

Every FAE cycle exchanges humid tent air for drier room air. This reliably drops RH 2–5% depending on room conditions. The humidity loop will respond and run the fogger to compensate. These two loops are **not independent** — shortening the CO₂ band (triggering FAE more frequently) directly increases fogger duty cycle.

---

## Timer Scheduler

**Grow lights** and **UVC reservoir lights** are time-based — no sensor feedback.

**Grow lights** follow a daily on/off window defined in minutes from midnight UTC:

```
ON  at LIGHTS_ON_MINUTE  (default 480  = 08:00 UTC)
OFF at LIGHTS_OFF_MINUTE (default 1200 = 20:00 UTC)
```

**UVC lights** follow a repeating duty cycle (not a wall-clock schedule):

```
ON  for UVC_ON_DURATION_MIN  (default 60 min)
OFF for UVC_OFF_DURATION_MIN (default 240 min)
→ repeats: 1 hour on, 4 hours off, continuously
```

UVC has a **10-second boot guard** (`BOOT_LOCK_MS` 5 s + `UVC_EXTRA_GUARD_MS` 5 s) that keeps the relay locked off after power-on regardless of the schedule. This prevents a UV pulse during initial wiring verification.

---

## Water Level Loop

**Sensor:** Submersible hydrostatic pressure sensor → ADC (GPIO 34 / GPIO 7 on S3)
**Actuator:** Relay Ch7 — Top-off pump (optional — relay wired at build time, pump not installed in base build)

```
if water_level_pct < WATER_LEVEL_LOW_PCT   → turn pump ON
if water_level_pct > WATER_LEVEL_HIGH_PCT  → turn pump OFF
```

The ADC reading passes through a 32-sample rolling average (`WATER_LEVEL_SAMPLES`) to filter out electrical noise from the fogger motor. The raw millivolt reading is mapped to a 0–100% range using two calibration constants that **must be set for your specific sensor** (see ADC Calibration below).

---

## Tuning Parameters

Parameters split into two tiers: **runtime** (changeable via API, persisted to NVS, survive reboot) and **compile-time** (require a firmware reflash).

### Runtime Parameters (NVS)

Set via `POST /api/config` or the web dashboard config panel.

#### Humidity

| Parameter | API field | Default | Effect |
|-----------|-----------|---------|--------|
| RH on threshold | `rh_on_pct` | `85.0` % | Fogger turns ON when aggregate RH drops below this |
| RH hysteresis | `rh_hysteresis` | `2.0` % | Fogger turns OFF at `rh_on_pct + hysteresis` (default: 87%) |
| RH aggregation | `rh_aggregation` | `0` (AVERAGE) | `0`=average, `1`=min, `2`=max of shelf sensors |

**When to adjust:** If per-shelf VPD data shows one shelf consistently drier than the others, switch `rh_aggregation` to `1` (MIN) — the driest shelf then drives the fogger. Raise `rh_on_pct` if you are seeing bacterial blotch; lower it if you see cracked caps or aborted pins at high VPD readings.

#### CO₂ / FAE

| Parameter | API field | Default | Effect |
|-----------|-----------|---------|--------|
| CO₂ on threshold | `co2_on_ppm` | `950` ppm | FAE fans turn ON above this |
| CO₂ off threshold | `co2_off_ppm` | `800` ppm | FAE fans turn OFF below this |

The band width (on − off = 150 ppm default) controls FAE cycle frequency. **If FAE is cycling too frequently and the humidity loop is struggling to keep up**, widen the band (e.g. 1000 / 700) to reduce fan duty cycle. **If CO₂ is climbing high between cycles**, narrow the band or lower the on threshold.

#### Timers

| Parameter | API field | Default | Effect |
|-----------|-----------|---------|--------|
| Lights on | `timer.lights_on_minute` | `480` (08:00 UTC) | Daily lights-on time in minutes from midnight |
| Lights off | `timer.lights_off_minute` | `1200` (20:00 UTC) | Daily lights-off time |
| UVC on duration | `timer.uvc_on_min` | `60` min | How long UVC stays on per cycle |
| UVC off duration | `timer.uvc_off_min` | `240` min | How long UVC stays off between cycles |

Adjust lights times to match local civil time — the firmware uses UTC internally (NTP-synced), so set `timezone` to your POSIX TZ string to make the UI display local time correctly.

#### Water Level

| Parameter | API field | Default | Effect |
|-----------|-----------|---------|--------|
| Pump on threshold | `water_low_pct` | `20.0` % | Pump activates when reservoir drops below this |
| Pump off threshold | `water_high_pct` | `80.0` % | Pump cuts off when reservoir reaches this |

#### ADC Calibration (mandatory for water level accuracy)

| Parameter | API field | Default | Effect |
|-----------|-----------|---------|--------|
| ADC zero (empty) | `adc_water_min_mv` | `200` mV | Voltage reading at 0% water level |
| ADC full | `adc_water_max_mv` | `3100` mV | Voltage reading at 100% water level |

**These defaults are estimates.** The actual output of a 4–20 mA pressure sensor through a shunt resistor varies by sensor batch and resistor tolerance. Calibrate in-situ: measure the ADC millivolt output with the tank empty and with the tank full (use `/api/status` to read raw ADC values), then set these two parameters to match. Without calibration, water level percentages will be wrong.

---

### Compile-Time Parameters

These require editing `include/config.h` and reflashing firmware. They are not exposed via the API.

| Parameter | Default | What it protects |
|-----------|---------|-----------------|
| `HUMIDITY_COOLDOWN_MS` | 30 000 ms | Minimum time between fogger state changes. Prevents relay chatter and motor cycling near the setpoint. |
| `FAE_MIN_RUN_MS` | 60 000 ms | Minimum FAE fan run per cycle. Ensures a complete air exchange before the fans can cut off. |
| `WATER_LEVEL_SAMPLES` | 32 | Rolling average depth for ADC. Higher = smoother reading, slower to respond to actual level changes. |
| `SHT45_HEATER_INTERVAL_MS` | 3 600 000 ms (60 min) | Frequency of SHT45 on-chip heater pulse. Corrects for humidity creep in continuous high-RH environments. Do not reduce below 30 min. |
| `SENSOR_STALE_MS` | 30 000 ms | Reading age before it is flagged stale and excluded from aggregation. |
| `BOOT_LOCK_MS` | 5 000 ms | All relays held OFF for this duration after power-on. Safety-critical. Do not reduce. |
| `UVC_EXTRA_GUARD_MS` | 5 000 ms | Additional delay before UVC relay is allowed to energise. Combined with `BOOT_LOCK_MS` = 10 s total. Safety-critical. Do not reduce. |

---

## Tuning Philosophy

The defaults are calibrated for gourmet species (oyster, shiitake, lion's mane) at typical basement temperatures (18–22°C). Most parameters will not need adjustment.

**Before changing anything:** run for at least one full flush (pinning through harvest) and collect per-shelf VPD and RH data across the cycle. Distinguish a real pattern from a one-off event. The three-shelf sensor array exists precisely to give you the data to make this judgement.

**Change one parameter at a time.** The humidity and CO₂ loops interact — a change to the CO₂ band affects fogger duty cycle. Changing both simultaneously makes it impossible to understand which change had which effect.

**Do not tighten the timing parameters** (`HUMIDITY_COOLDOWN_MS`, `FAE_MIN_RUN_MS`) to make the control feel more responsive. These are anti-oscillation safeguards. The cost of a relay burning out or a fogger motor cycling to failure outweighs any marginal improvement in setpoint tracking.
