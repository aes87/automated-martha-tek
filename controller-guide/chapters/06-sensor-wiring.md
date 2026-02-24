# Chapter 06 — Sensor Wiring

**What you'll do:** Wire all sensors to the ESP32-S3: I2C multiplexer, CO2 sensor,
three temp/RH sensors, spectral light sensor, five substrate temp probes, and the
water level sensor.

**Prerequisites:** Chapter 05 complete (ESP32-S3 mounted). All sensor breakout
boards, DS18B20 probes, DFRobot KIT0139 kit, 2.2 kΩ resistors ×2, 100 nF
capacitors ×2, 22 AWG stranded wire, soldering iron.

> ⚠️ **SAFETY:** All wiring in this chapter is low-voltage (3.3V/5V DC and 12V DC).
> No mains voltage. Keep the enclosure unplugged.

---

## 6a — I2C Bus Fundamentals

> **[?] I2C:** I2C (Inter-Integrated Circuit) is a two-wire communication protocol.
> It uses two wires shared by all devices on the bus:
> - **SDA** (Serial Data) — carries the data
> - **SCL** (Serial Clock) — the timing signal
>
> Every device on the bus has a unique address (a short number like `0x44` or `0x61`).
> The ESP32 announces an address to talk to a specific device; only that device
> responds. Multiple devices share the same two wires — the address system prevents
> collisions.
>
> Important for this build: **all I2C devices must run at 3.3V**, not 5V. The ESP32-S3
> I2C pins are 3.3V logic. The Adafruit breakout boards for SCD30, SHT45, AS7341, and
> TCA9548A all accept a wide VIN (3.3V–5V) but their I2C lines must be at 3.3V.
> Power all breakout boards from the ESP32's `3V3` pin, not from the 5V PSU rail.

**I2C device addresses in this build:**

| Device | Address |
|--------|---------|
| TCA9548A (I2C multiplexer) | `0x70` |
| SHT45 ×3 (temp/RH, via mux channels 0/1/2) | `0x44` (each) |
| SCD30 (CO2) | `0x61` |
| AS7341 (light spectrum) | `0x39` |

**Bus wiring overview:**

```
ESP32 GPIO 21 (SDA) ───────┬── TCA9548A SDA
ESP32 GPIO 9  (SCL) ───────┼── TCA9548A SCL
                            ├── SCD30 SDA/SCL    (direct on main bus, addr 0x61)
                            └── AS7341 SDA/SCL   (direct on main bus, addr 0x39)

ESP32 3V3 ─────────────────┬── TCA9548A VIN
                            ├── SCD30 VIN
                            └── AS7341 VIN

ESP32 GND ───────────────── all sensor GND
```

Use 22 AWG wire. Colour-code consistently: white = SDA, yellow = SCL (or choose
your own scheme and label each wire at both ends).

---

## 6b — TCA9548A I2C Multiplexer

The TCA9548A is an 8-channel I2C switch. Three SHT45 sensors all have address
`0x44` — they would collide on a direct bus. The multiplexer puts each on its own
channel so the ESP32 can talk to them one at a time.

**Wiring:**

| TCA9548A pin | Connect to |
|-------------|-----------|
| VIN | ESP32 `3V3` |
| GND | ESP32 GND |
| SDA | ESP32 `GPIO 21` (main bus SDA) |
| SCL | ESP32 `GPIO 9` (main bus SCL) |
| A0 | GND (sets I2C address bit 0 = 0) |
| A1 | GND (sets I2C address bit 1 = 0) |
| A2 | GND (sets I2C address bit 2 = 0) |

With A0/A1/A2 all tied to GND, the TCA9548A's I2C address is **`0x70`**.

Channels 0, 1, and 2 on the TCA9548A will connect to the three SHT45 sensors
in Steps 6c/6d.

**✓ Check:** TCA9548A wired to main I2C bus. A0/A1/A2 all tied to GND. VIN
connected to 3.3V, not 5V.

---

## 6c — Sensirion SCD30 (CO2 / Temp / RH)

The SCD30 measures CO2, temperature, and relative humidity. It sits directly on
the main I2C bus (address `0x61` does not conflict with anything else).

**Wiring:**

| SCD30 pin | Connect to |
|-----------|-----------|
| VIN | ESP32 `3V3` |
| GND | ESP32 GND |
| SDA | ESP32 `GPIO 21` (main bus) |
| SCL | ESP32 `GPIO 9` (main bus) |
| SEL | GND (selects I2C mode — required) |

> **[?] SEL pin:** The SCD30 supports two interfaces: I2C and Modbus. The SEL
> pin selects which one. Tie it to GND for I2C mode. On the Adafruit SCD30 breakout
> (Adafruit #4867), this is handled by a default solder jumper that you don't need
> to touch. If you have a bare SCD30 module (not the Adafruit breakout), you must
> explicitly wire SEL to GND.

**Placement note:** The SCD30 will eventually live inside the tent at mid-height,
mounted on the back wall with a small plastic baffle to deflect direct fog. For
now, just bring the cable through a PG9 gland and leave enough cable length to
reach the tent mounting position. Coil the excess inside the enclosure.

**✓ Check:** SCD30 wired to main I2C bus. SEL confirmed tied to GND. Cable has
enough length to reach mounting position in tent.

---

## 6d — Sensirion SHT45 ×3 (Temp / RH per Shelf)

Three SHT45 sensors measure temperature and relative humidity at three shelf heights
(top, middle, bottom). They all share I2C address `0x44`, so each connects to a
separate channel of the TCA9548A multiplexer.

**Channel assignment:**

| TCA9548A channel | SHT45 sensor | Tent position |
|-----------------|-------------|---------------|
| Channel 0 | SHT45 #1 | Top shelf height |
| Channel 1 | SHT45 #2 | Middle shelf height |
| Channel 2 | SHT45 #3 | Bottom shelf height |

**Wiring for each SHT45:**

| SHT45 pin | Connect to |
|-----------|-----------|
| VIN | TCA9548A channel VCC (or direct to ESP32 `3V3`) |
| GND | ESP32 GND |
| SDA | TCA9548A channel N SDA |
| SCL | TCA9548A channel N SCL |

The SHT45 Adafruit breakout (#6174) has VIN, GND, SDA, SCL labelled on the board.

**Label each sensor now** — before running cables into the tent — with a wrap of
label tape around the cable near the sensor: "SHT45 TOP", "SHT45 MID", "SHT45
BOT". Once they're inside a tent with cables running along the frame, telling them
apart from the outside is very difficult.

**✓ Check:** All three SHT45 sensors wired to TCA9548A channels 0, 1, and 2.
Each sensor labelled at the cable. All VIN connections to 3.3V, not 5V.

---

## 6e — AS7341 (Spectral Light Sensor)

The AS7341 measures light in 11 channels across 415–680 nm plus NIR. It lives on
the main I2C bus (address `0x39`).

> ⚠️ **SAFETY:** The AS7341 is **not 5V tolerant** on its I2C pins or VIN. It must
> be powered from 3.3V only. Connecting it to 5V will destroy it.

**Wiring:**

| AS7341 pin | Connect to |
|-----------|-----------|
| VIN | ESP32 `3V3` (3.3V only — not 5V) |
| GND | ESP32 GND |
| SDA | ESP32 `GPIO 21` (main bus) |
| SCL | ESP32 `GPIO 9` (main bus) |

**Placement note:** The AS7341 mounts at the top of the tent, facing downward,
to measure the light reaching the fruiting blocks. Run its cable through a PG9
gland. Leave enough cable length for mounting at the top of the tent frame.

**✓ Check:** AS7341 wired to main I2C bus with 3.3V VIN (not 5V). Cable has
enough length for tent ceiling mounting.

---

## 6f — DS18B20 ×5 (Substrate Temperature, 1-Wire)

Five DS18B20 waterproof probes measure substrate temperature — one per shelf.
All five connect to a single GPIO pin using the 1-Wire daisy-chain protocol.

> **[?] 1-Wire bus:** A single-wire protocol where multiple sensors share one data
> wire plus power and ground. Each DS18B20 has a factory-programmed unique 64-bit
> ID burned into its chip, so the ESP32 can address each probe individually even
> though they're all on the same wire. It's like a party line telephone where each
> person has their own extension number.

### Assemble the 1-Wire Pull-Up Circuit

Before wiring the probes, build the pull-up circuit. You need:
- 1× 2.2 kΩ resistor
- 1× 100 nF (0.1 µF) ceramic capacitor

```
ESP32 3V3 ──────────── 2.2kΩ ──────┬──── to DS18B20 data line
                                    │
                                  100nF
                                    │
ESP32 GND ─────────────────────────┘
```

> **[?] Why 2.2 kΩ and not 4.7 kΩ?** The DS18B20 datasheet recommends 4.7 kΩ
> for a single sensor. With 5 sensors on 5–8 m of cable, the combined capacitance
> of the cable slows the rise time of the data line. A 4.7 kΩ pull-up may cause
> intermittent read failures on long chains. 2.2 kΩ drives the bus harder and
> stays reliable. Use 1 kΩ if dropouts occur on very long or aged cables.

> **[?] Why a 100 nF cap?** The data cable to the tent probes acts as an antenna
> for electrical noise, especially with relay coils switching nearby. The capacitor
> sits at the pull-up junction (where the resistor meets the data line) and filters
> out brief voltage spikes that could corrupt 1-Wire data packets. Place it as close
> to `GPIO 4` as practical — on a small piece of stripboard, or between the pins of
> the ESP32's header.

**Solder the pull-up and capacitor** on a small piece of stripboard or directly
to the sensor's junction point. The resistor one end goes to ESP32 `3V3`; the
other end connects to both the data line and one leg of the capacitor. The other
leg of the capacitor goes to GND.

### Wire the DS18B20 Probes

Each DS18B20 waterproof probe has three wires:
- **Red** — VCC (power, connect to ESP32 `3V3`)
- **Black** — GND
- **Yellow** — Data (connect to `GPIO 4` via the pull-up junction)

**Daisy chain wiring:**

```
ESP32 GPIO 4 ─── pull-up junction ─── Data (yellow) of all 5 probes
ESP32 3V3 ────────────────────────── VCC (red) of all 5 probes
ESP32 GND ────────────────────────── GND (black) of all 5 probes
```

The three wires run from the enclosure through a PG9 gland, down to the tent,
and along each shelf where a probe rests in or against a fruiting block.

**Label each probe before installing:**
Wrap a piece of label tape around each probe cable near the sensor end:
"DS18 S1", "DS18 S2", "DS18 S3", "DS18 S4", "DS18 S5". Once inside the tent, the
firmware will identify them by their ROM addresses — but physical labels are your
safety net if cables get mixed up.

**✓ Check:** Pull-up resistor (2.2 kΩ) wired between 3V3 and `GPIO 4`. Decoupling
cap (100 nF) wired between pull-up junction and GND. All 5 probe data wires tied
together on `GPIO 4` data line. All probes labelled S1–S5.

---

## 6g — Water Level Sensor (DFRobot KIT0139)

The KIT0139 is a submersible hydrostatic pressure sensor with a matched Gravity
4-20mA-to-voltage converter board. The converter board includes overvoltage
protection for the ESP32 ADC input.

> ⚠️ **SAFETY:** The sensor body connects to the 12V PSU supply. The Gravity
> converter board output connects to the ESP32 ADC at 3.3V. Do not connect the
> 12V sensor supply directly to the ESP32 GPIO — route it through the converter
> board only.

**Wire the sensor to the 12V PSU (sensor supply):**

The submersible probe has two wires:
| Wire colour (typical) | Connect to |
|-----------------------|-----------|
| Brown / Red | 12V PSU positive output (+) |
| Blue / Black | 12V PSU GND (common GND) |

**Wire the Gravity converter board:**

| Converter board pin | Connect to |
|--------------------|-----------|
| VCC | ESP32 `3V3` |
| GND | ESP32 GND (common GND) |
| OUT / Signal | ESP32 `GPIO 7` (ADC1_CH6) |

The converter board's sensor input terminals (if separate from the above) connect
to the sensor signal and GND lines. Follow the KIT0139 wiring diagram included
with the kit.

> **[?] 4–20 mA current loop:** The sensor produces a current that varies from 4 mA
> (when submerged in an empty reservoir) to 20 mA (when water pressure from a full
> reservoir is sensed). The converter board turns this current into a 0–3.3V voltage
> the ESP32 can read. 0 V = empty, 3.3 V = full (approximately). The ESP32 ADC then
> converts this to a digital number the firmware maps to a percentage.

**Route the sensor cable:**
- Pass the sensor cable through a PG9 cable gland in the LV zone of the enclosure
- The converter board mounts inside the enclosure (in the LV zone, near the ESP32)
- The sensor probe drops to the floor of the 19-gallon reservoir under the tent

**✓ Check:** 12V sensor supply connected to converter board sensor input (or direct
to probe as per KIT0139 diagram). Converter board OUT connected to `GPIO 7`. Converter
board VCC at 3.3V and GND at common ground. Cable routed through PG9 gland.

---

## Chapter 06 Checkpoint

- [ ] TCA9548A wired to main I2C bus (GPIO 21 / GPIO 9); A0/A1/A2 → GND; VIN → 3V3
- [ ] SCD30 wired to main I2C bus; SEL → GND; VIN → 3V3
- [ ] AS7341 wired to main I2C bus; VIN → 3V3 (confirmed not 5V)
- [ ] Three SHT45 sensors wired to TCA9548A channels 0, 1, and 2 respectively
- [ ] Each SHT45 labelled (TOP / MID / BOT) at the cable
- [ ] DS18B20 pull-up resistor (2.2 kΩ) installed between GPIO 4 and 3V3
- [ ] DS18B20 decoupling cap (100 nF) installed at pull-up junction to GND
- [ ] All 5 DS18B20 probes daisy-chained on GPIO 4 data line
- [ ] Each DS18B20 probe labelled S1–S5 at the cable
- [ ] KIT0139 converter board wired: OUT → GPIO 7; VCC → 3V3; GND → GND
- [ ] KIT0139 sensor supply wired to 12V PSU output
- [ ] All sensor cables routed through appropriate PG9 cable glands
- [ ] All I2C devices powered from 3V3, not from 5V PSU rail
- [ ] Common GND shared: 5V PSU −, 12V PSU −, and ESP32 GND all tied together
