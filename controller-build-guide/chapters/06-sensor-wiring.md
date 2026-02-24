# Chapter 06 — Sensor Wiring

**What you'll do:** Wire all sensors to the ESP32-S3: I2C multiplexer, CO2 sensor,
three temp/RH sensors, spectral light sensor, five substrate temp probes, and the
water level sensor.

**Prerequisites:** Chapter 05 complete (ESP32-S3 mounted). All sensor breakout
boards, DS18B20 probes, DFRobot KIT0139 kit, 2.2 kΩ resistors ×2, 100 nF
capacitors ×2, 22 AWG stranded wire, soldering iron.

> ⚠️ **SAFETY:** All wiring in this chapter is low-voltage (3.3V/5V and 12V DC).
> No mains voltage. Keep the enclosure unplugged.

---

## 6a — I2C Bus Fundamentals

<details>
<summary><strong>[?] I2C:</strong></summary>

A two-wire communication protocol. All devices share two wires:
- **SDA** (Serial Data) — carries the data
- **SCL** (Serial Clock) — the timing signal

Each device has a unique address (e.g. `0x44`, `0x61`). The ESP32 addresses a
specific device; only that device responds.

All I2C devices in this build run at **3.3V**. Power all breakout boards from the
ESP32's `3V3` pin, not from the 5V PSU rail.

</details>

**I2C device addresses:**

| Device | Address |
|--------|---------|
| TCA9548A (I2C multiplexer) | `0x70` |
| SHT45 ×3 (via mux channels 0/1/2) | `0x44` each |
| SCD30 (CO2) | `0x61` |
| AS7341 (light spectrum) | `0x39` |

**Main bus wiring:**

```
ESP32 GPIO 21 (SDA) ───────┬── TCA9548A SDA
ESP32 GPIO 9  (SCL) ───────┼── TCA9548A SCL
                            ├── SCD30 SDA/SCL  (addr 0x61)
                            └── AS7341 SDA/SCL (addr 0x39)

ESP32 3V3 ─────────────────┬── TCA9548A VIN
                            ├── SCD30 VIN
                            └── AS7341 VIN

ESP32 GND ───────────────── all sensor GND
```

Use 22 AWG wire. Colour-code consistently (suggestion: white = SDA, yellow = SCL).
Label each wire at both ends.

---

## 6b — TCA9548A I2C Multiplexer

The three SHT45 sensors all share I2C address `0x44` and would conflict on a direct
bus. The TCA9548A places each on its own channel.

**Wiring:**

| TCA9548A pin | Connect to |
|-------------|-----------|
| VIN | ESP32 `3V3` |
| GND | ESP32 GND |
| SDA | ESP32 `GPIO 21` |
| SCL | ESP32 `GPIO 9` |
| A0 | GND |
| A1 | GND |
| A2 | GND |

A0/A1/A2 all tied to GND sets the TCA9548A's I2C address to **`0x70`**.

**✓ Check:** TCA9548A wired to main I2C bus. A0/A1/A2 all at GND. VIN at 3.3V.

---

## 6c — Sensirion SCD30 (CO2 / Temp / RH)

The SCD30 sits directly on the main I2C bus (address `0x61`).

**Wiring:**

| SCD30 pin | Connect to |
|-----------|-----------|
| VIN | ESP32 `3V3` |
| GND | ESP32 GND |
| SDA | ESP32 `GPIO 21` |
| SCL | ESP32 `GPIO 9` |
| SEL | GND (selects I2C mode) |

<details>
<summary><strong>[?] SEL pin:</strong></summary>

Ties LOW for I2C mode. On the Adafruit SCD30 breakout this is
handled by a default solder jumper — no action needed. On a bare SCD30 module,
wire SEL explicitly to GND.

</details>

Route the SCD30 cable through a PG9 gland with enough length to reach its mounting
position inside the tent (mid-height, back wall).

**✓ Check:** SCD30 wired to main I2C bus. SEL confirmed at GND. Cable has
sufficient length for tent mounting.

---

## 6d — Sensirion SHT45 ×3 (Temp / RH per Shelf)

Three SHT45 sensors measure temp/RH at top, middle, and bottom shelf heights. Each
connects to a separate TCA9548A channel.

**Channel assignment:**

| TCA9548A channel | Sensor | Tent position |
|-----------------|--------|---------------|
| Channel 0 | SHT45 #1 | Top shelf |
| Channel 1 | SHT45 #2 | Middle shelf |
| Channel 2 | SHT45 #3 | Bottom shelf |

**Wiring for each SHT45:**

| SHT45 pin | Connect to |
|-----------|-----------|
| VIN | ESP32 `3V3` |
| GND | ESP32 GND |
| SDA | TCA9548A channel N SDA |
| SCL | TCA9548A channel N SCL |

**Label each sensor before running cables into the tent:** wrap label tape around
the cable near the sensor — "SHT45 TOP", "SHT45 MID", "SHT45 BOT".

**✓ Check:** SHT45s wired to TCA9548A channels 0, 1, and 2. Each sensor labelled.
All VIN connections at 3.3V.

---

## 6e — AS7341 (Spectral Light Sensor)

The AS7341 sits on the main I2C bus (address `0x39`).

> ⚠️ **SAFETY:** The AS7341 is **not 5V tolerant**. VIN must be 3.3V. Connecting
> it to 5V will destroy it.

**Wiring:**

| AS7341 pin | Connect to |
|-----------|-----------|
| VIN | ESP32 `3V3` (3.3V only) |
| GND | ESP32 GND |
| SDA | ESP32 `GPIO 21` |
| SCL | ESP32 `GPIO 9` |

Route cable through a PG9 gland with enough length to mount at the top of the tent
facing downward.

**✓ Check:** AS7341 wired to main I2C bus with 3.3V VIN confirmed.

---

## 6f — DS18B20 ×5 (Substrate Temperature, 1-Wire)

Five probes connect to a single GPIO using the 1-Wire daisy-chain protocol.

<details>
<summary><strong>[?] 1-Wire bus:</strong></summary>

A single-wire protocol where multiple sensors share one data
wire plus power and ground. Each DS18B20 has a factory-burned unique 64-bit ID,
so the ESP32 addresses each probe individually on the same wire.

</details>

### Assemble the Pull-Up Circuit

You need one 2.2 kΩ resistor and one 100 nF ceramic capacitor:

```
ESP32 3V3 ──── 2.2kΩ ──────┬──── DS18B20 data line (GPIO 4)
                            │
                          100nF
                            │
ESP32 GND ─────────────────┘
```

<details>
<summary><strong>[?] Why 2.2 kΩ?</strong></summary>

With 5 sensors on several metres of cable, the cable
capacitance slows the rise time of the data line. 2.2 kΩ drives the bus reliably
across the full chain.

</details>

<details>
<summary><strong>[?] Why a 100 nF cap?</strong></summary>

The data cable acts as an antenna for noise from nearby
relay coils. The capacitor at the pull-up junction filters voltage spikes that
could corrupt 1-Wire data. Place it close to `GPIO 4`.

</details>

Solder the pull-up resistor and cap on a small piece of stripboard or at the GPIO 4
header. Resistor: 3V3 to data line junction. Cap: data line junction to GND.

### Wire the DS18B20 Probes

Each probe has three wires:
- **Red** — VCC → ESP32 `3V3`
- **Black** — GND
- **Yellow** — Data → `GPIO 4` (via pull-up junction)

All five probes daisy-chain on the same three wires. Route through a PG9 gland.

**Label each probe before installing:** "DS18 S1" through "DS18 S5" at the sensor
end of each cable.

**✓ Check:** Pull-up resistor (2.2 kΩ) between 3V3 and GPIO 4. Decoupling cap
(100 nF) at pull-up junction to GND. All 5 probes on GPIO 4 data line. All probes
labelled.

---

## 6g — Water Level Sensor (DFRobot KIT0139)

The KIT0139 kit includes the submersible sensor and the Gravity 4-20mA-to-voltage
converter board. The converter board provides overvoltage protection for the ESP32
ADC pin.

> ⚠️ **SAFETY:** The sensor body runs on 12V. The converter board output connects
> to the ESP32 at 3.3V. Never connect the 12V sensor supply directly to an ESP32
> GPIO.

**Sensor to 12V PSU:**

| Sensor wire | Connect to |
|-------------|-----------|
| Brown / Red | 12V PSU + output |
| Blue / Black | 12V PSU − (common GND) |

**Gravity converter board:**

| Converter pin | Connect to |
|--------------|-----------|
| VCC | ESP32 `3V3` |
| GND | ESP32 GND |
| OUT / Signal | ESP32 `GPIO 7` |

Connect the sensor signal and return wires to the converter board's sensor input
terminals per the KIT0139 wiring diagram included with the kit.

<details>
<summary><strong>[?] 4–20 mA current loop:</strong></summary>

The sensor outputs 4 mA at minimum depth and 20 mA
at maximum. The converter board converts this current to 0–3.3V for the ESP32 ADC.

</details>

The converter board mounts in the LV zone. The sensor probe drops to the floor of
the reservoir. Route cable through a PG9 gland.

**✓ Check:** 12V sensor supply wired through converter board to GPIO 7. Converter
board VCC at 3.3V, GND at common ground. Cable through PG9 gland.

---

## Chapter 06 Checkpoint

- [ ] TCA9548A: VIN→3V3; SDA→GPIO 21; SCL→GPIO 9; A0/A1/A2→GND
- [ ] SCD30: VIN→3V3; SDA/SCL on main bus; SEL→GND
- [ ] AS7341: VIN→3V3 (confirmed, not 5V); SDA/SCL on main bus
- [ ] SHT45 #1 → TCA9548A ch0; labelled TOP
- [ ] SHT45 #2 → TCA9548A ch1; labelled MID
- [ ] SHT45 #3 → TCA9548A ch2; labelled BOT
- [ ] DS18B20 pull-up: 2.2 kΩ between GPIO 4 and 3V3
- [ ] DS18B20 decoupling cap: 100 nF at pull-up junction to GND
- [ ] All 5 DS18B20 probes on GPIO 4; each labelled S1–S5
- [ ] KIT0139: OUT→GPIO 7; VCC→3V3; sensor supply from 12V PSU
- [ ] All sensor cables through PG9 glands
- [ ] All I2C devices powered from 3V3, not 5V PSU rail
- [ ] Common GND: 5V PSU −, 12V PSU −, and ESP32 GND all tied together

---

[← Ch 05 — ESP32-S3 Setup](05-esp32-s3-setup.md)  ·  [Ch 07 — Failsafe Panel Switches →](07-failsafe-panel.md)
