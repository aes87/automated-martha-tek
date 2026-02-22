# DIY Martha Tent Controller — Full Build Guide

A highly-featured ESP32-based controller for a Martha tent fruiting chamber. Replaces ~$233 worth of off-the-shelf CO2 and humidity controllers with a single board that gives you more accurate sensors, full data logging, and a web dashboard.

**What you replace:** dccrens' CO2 controller ($161) + Inkbird IHC200 humidity controller ($42) + Inkbird IBS-TH2 Plus monitor ($30).

**What you get instead:** real-time CO2, per-shelf humidity and temperature gradients, substrate temperature per shelf, spectral light monitoring, continuous water level, VPD calculation, web dashboard, and optional automatic reservoir top-off.

**Hardware cost: ~$270–285** (base build) — roughly the same price for dramatically more capability.

> This guide covers hardware only. Software/firmware is covered separately.
>
> Reference build credit: u/mettalmag (r/MushroomGrowers, Jan 2025) — adapted from a 200sqm commercial greenhouse to a home Martha tent.
> Original post: https://www.reddit.com/r/MushroomGrowers/comments/1rao1ms/

---

## ⚠️ Safety Warning — Mains Voltage

This build switches 120V AC (or 240V depending on region) mains loads. **Mains voltage can kill.** Before you start:

- All mains wiring must be inside the sealed half of the enclosure, physically separated from the low-voltage electronics
- Use 14 AWG minimum wire for mains runs; 18 AWG minimum for load leads under 10A
- Use insulated ferrules or properly rated terminals on all mains connections — no bare wire ends in screw terminals
- Always power down and unplug before touching any wiring
- If you are not comfortable with mains wiring, have a qualified electrician do the line-side portion
- Ground the enclosure

---

## How It Works

The ESP32 reads all sensors and drives an 8-channel relay module that switches the tent's loads. The firmware runs two control loops:

- **Humidity loop:** SHT45 sensors measure RH → fogger + tub fan relay fires when RH drops below 85%
- **CO2/FAE loop:** SCD30 measures CO2 → exhaust + intake fan relays fire when CO2 exceeds 950 ppm

Everything else (UVC, lights) runs on timers. All data streams to a web dashboard in real time. Physical failsafe switches on the panel face let you control any load group manually, independent of the ESP32.

```
                    ┌─────────────────────────────────────┐
    SENSORS         │           ESP32 DevKit V1            │    RELAY OUTPUTS
                    │                                       │
  SCD30 (CO2) ─────┤ I2C (GPIO 21/22)     GPIO 16 ├──── Ch1: Fogger
  SHT45 ×3 ────────┤ via TCA9548A mux     GPIO 17 ├──── Ch2: Tub fan
  AS7341 (light) ──┤                      GPIO 18 ├──── Ch3: Exhaust fan
                   │                      GPIO 19 ├──── Ch4: Intake fan
  DS18B20 ×5 ──────┤ 1-Wire (GPIO 4)      GPIO 23 ├──── Ch5: UVC lights
                   │                      GPIO 25 ├──── Ch6: Grow lights
  Water level ─────┤ ADC (GPIO 34)        GPIO 26 ├──── Ch7: Pump (optional)
                   │                      GPIO 27 ├──── Ch8: Spare
                    └─────────────────────────────────────┘
                              │
                     Failsafe panel switches
                     (bypass relay board in MANUAL mode)
```

![Controller Architecture](diy-controller-wiring.svg)

---

## Parts List

### Core Electronics

| Component | Part | Source | Qty | Est. Cost |
|-----------|------|--------|-----|-----------|
| Microcontroller | ESP32 DevKit V1 | [Amazon](https://www.amazon.com/s?k=ESP32+DevKit+V1+38pin) | 1 | ~$10 |
| Relay module | 8-channel 5V opto-isolated relay | [Amazon](https://www.amazon.com/s?k=8+channel+relay+module+optocoupler+5v) | 1 | ~$10 |
| CO2 sensor | Sensirion SCD30 | [Adafruit #4867](https://www.adafruit.com/product/4867) | 1 | $58.95 |
| Temp/RH sensor | Sensirion SHT45 + PTFE membrane | [Adafruit #6174](https://www.adafruit.com/product/6174) | 3 | $40.50 |
| I2C multiplexer | TCA9548A 8-channel | [Adafruit #2717](https://www.adafruit.com/product/2717) | 1 | $6.95 |
| Water level sensor | Submersible pressure sensor + converter | [DFRobot KIT0139](https://www.dfrobot.com/product-1863.html) | 1 | ~$44 |
| Substrate temp | DS18B20 waterproof probe (1m cable) | [Adafruit #381](https://www.adafruit.com/product/381) | 5 | ~$50 |
| Light sensor | AS7341 11-channel spectral | [Adafruit #4698](https://www.adafruit.com/product/4698) | 1 | ~$12 |

### Power

| Component | Spec | Source | Qty | Est. Cost |
|-----------|------|--------|-----|-----------|
| 5V PSU | 5V 2A, DIN rail or panel mount | [Amazon](https://www.amazon.com/s?k=5v+2a+power+supply+DIN+rail) | 1 | ~$10 |
| 12V PSU | 12V 1A, DIN rail or panel mount | [Amazon](https://www.amazon.com/s?k=12v+1a+power+supply+DIN+rail) | 1 | ~$10 |

### Enclosure

| Component | Spec | Source | Qty | Est. Cost |
|-----------|------|--------|-----|-----------|
| IP65 enclosure | Min 250×200×100mm | [Amazon](https://www.amazon.com/s?k=IP65+waterproof+enclosure+250x200mm) | 1 | ~$20–30 |
| DIN rail (35mm) | Cut to enclosure width | Hardware store | 1 | ~$5 |
| Cable glands (PG9/PG11/PG16) | For sensor cables, power in, load out | [Amazon](https://www.amazon.com/s?k=PG11+cable+gland+nylon) | assorted | ~$8 |

### Panel & Wiring

| Component | Spec | Source | Qty | Est. Cost |
|-----------|------|--------|-----|-----------|
| Panel switches | DPDT toggle (master) + SPST toggle ×4 | [Amazon](https://www.amazon.com/s?k=DPDT+toggle+switch+panel+mount) | 5 | ~$12 |
| Wire | 22 AWG solid/stranded (low voltage); 14 AWG (mains) | Hardware store | — | ~$10 |
| Wago 221 connectors | Push-in for mains terminals (safer than bare screws) | [Amazon](https://www.amazon.com/s?k=Wago+221+lever+connectors) | 1 pack | ~$10 |
| 4.7kΩ resistor | 1-Wire pull-up | Electronics supplier | 1 | <$1 |
| 150Ω resistor | Water level 4-20mA shunt | Electronics supplier | 1 | <$1 |
| Ferrule crimp kit | For clean screw terminal ends | [Amazon](https://www.amazon.com/s?k=ferrule+crimp+kit+22AWG) | 1 | ~$10 |
| Terminal block strip | DIN rail mounted, 10-position | [Amazon](https://www.amazon.com/s?k=DIN+rail+terminal+block+10+position) | 2 | ~$8 |
| Fuse holder + fuses | Panel mount, 5A (mains side) | [Amazon](https://www.amazon.com/s?k=panel+mount+fuse+holder+5A) | 1 | ~$5 |

### Misc

| Component | Notes | Est. Cost |
|-----------|-------|-----------|
| Standoffs (M3, 10mm) | For mounting ESP32 + relay board to DIN rail or base | ~$3 |
| Zip ties / velcro | Cable management | ~$3 |
| Heat shrink tubing | Insulate splices and solder joints | ~$3 |
| Label maker tape or marker | Label every wire at both ends | ~$2 |

**Base build total: ~$280–300**

---

### Optional Add-on: Reservoir Top-Off Pump

> Relay Ch 7 and the 12V rail are pre-wired for this in the base build. Add the pump at any time — no enclosure rewiring needed.

| Component | Part | Source | Est. Cost |
|-----------|------|--------|-----------|
| 12V DC submersible pump | 3–5 L/min, compatible with tap water | [Amazon](https://www.amazon.com/s?k=12v+DC+submersible+water+pump+aquarium) | ~$10 |
| Food-safe silicone tubing | 3/8" or 1/2" ID | Hardware/aquarium store | ~$5 |
| Check valve | Prevents backflow into pump | [Amazon](https://www.amazon.com/s?k=aquarium+check+valve+3%2F8+inch) | ~$5 |
| **Add-on total** | | | **~$20** |

When added, the firmware monitors the water level sensor and triggers Ch 7 to top off the reservoir automatically.

---

## Tools Required

- Soldering iron + solder
- Wire strippers (22 AWG and 14 AWG)
- Ferrule crimping tool
- Multimeter (essential — test every connection before powering on)
- Drill + step bit (for enclosure cutouts)
- Screwdrivers (flathead + Phillips)
- Label maker or permanent marker

---

## Enclosure Layout

Use an IP65 box of at least 250×200×100mm. Divide it into two logical zones:

```
┌─────────────────────────────────────────────────────────────┐
│  MAINS ZONE (left 1/3)         │  LOW VOLTAGE ZONE (right)  │
│                                 │                             │
│  ┌──────────────┐               │  ┌─────────────────────┐   │
│  │  5V PSU      │               │  │  ESP32 DevKit V1    │   │
│  │  12V PSU     │               │  └─────────────────────┘   │
│  └──────────────┘               │                             │
│                                 │  ┌─────────────────────┐   │
│  ┌──────────────┐               │  │  8-ch Relay Module  │   │
│  │  Fuse block  │               │  └─────────────────────┘   │
│  │  Mains term- │               │                             │
│  │  inal blocks │               │  ┌─────────────────────┐   │
│  └──────────────┘               │  │  Sensor breakouts   │   │
│                                 │  │  (TCA9548A, SCD30,  │   │
│   ← Loads exit via              │  │   AS7341)           │   │
│     cable glands                │  └─────────────────────┘   │
│     on bottom edge              │                             │
│─────────────────────────────────│                             │
│  HIGH VOLTAGE — DO NOT TOUCH    │  Relay board bridges both  │
│  UNLESS FULLY UNPLUGGED         │  zones (coil LV, contact HV)│
└─────────────────────────────────────────────────────────────┘
                    │ Panel face (lid):
                    │  [MASTER AUTO/MAN] [HUMIDITY] [FAE] [UVC] [LIGHTS]
```

**Cable entry points:**
- Bottom or side of enclosure via cable glands
- Mains cable: PG16 gland
- Load cables (fans, fogger, lights): PG11 glands, one per load
- Sensor cables: PG9 glands (low voltage)
- Reservoir water level sensor cable: PG9 or PG11

Mount the relay board so its **load terminals (COM/NO/NC)** face the mains zone and its **signal header (IN1–IN8, VCC, GND)** faces the low-voltage zone. This is the natural bridge between the two halves.

---

## Wiring Guide

### Step 1 — Power Supply

Wire both PSUs from a single mains input cable (via the fuse block):

```
Mains In (fused 5A)
    ├── L (Hot) ──── 5V PSU L-in ──── 12V PSU L-in
    ├── N (Neutral) ─ 5V PSU N-in ──── 12V PSU N-in
    └── PE (Earth) ── 5V PSU PE ─────── 12V PSU PE ─── Enclosure chassis

5V PSU out:
    + ──── ESP32 VIN (5V) and Relay VCC/JD-VCC
    − ──── Common GND rail

12V PSU out:
    + ──── Water level sensor supply (+ terminal)
    − ──── Common GND rail (shared with 5V −)
```

> **Important:** Share a common negative/GND between both PSUs. This ensures the water level sensor's 4-20mA return current has a proper path and the ESP32 ADC reads correctly.

---

### Step 2 — Relay Module

#### Opto-isolation setup (important — don't skip)

Most 8-channel relay boards have a 3-pin header: **VCC**, **JD-VCC**, and **GND**, with a jumper between VCC and JD-VCC. **Remove this jumper.**

```
Relay board:
  VCC     ──── 3.3V from ESP32 (3V3 pin)   ← optocoupler logic side
  JD-VCC  ──── 5V from PSU                  ← relay coil side
  GND     ──── Common GND
```

This creates true electrical isolation between the ESP32 signal ground and the relay coil circuit, protecting the ESP32 if a relay coil spikes.

#### Signal connections (ESP32 → Relay)

These run through the **failsafe DPDT switch** in normal operation (see Step 4 before soldering these).

| Relay Channel | Load | ESP32 GPIO |
|---------------|------|------------|
| IN1 | Fogger | GPIO 16 |
| IN2 | Tub fan | GPIO 17 |
| IN3 | Exhaust fan | GPIO 18 |
| IN4 | Intake fan | GPIO 19 |
| IN5 | UVC lights | GPIO 23 |
| IN6 | Grow lights | GPIO 25 |
| IN7 | Top-off pump (optional) | GPIO 26 |
| IN8 | Spare | GPIO 27 |

> **Active LOW:** Most relay modules trigger when the IN pin is pulled LOW. The ESP32 GPIOs default HIGH at boot, which means all relays stay OFF during startup — safe default behaviour.

---

### Step 3 — Sensor Wiring

#### I2C bus

All I2C sensors share two wires: SDA (GPIO 21) and SCL (GPIO 22), plus 3.3V and GND. Run a 4-wire bus from the ESP32 to each breakout board.

```
ESP32 GPIO 21 (SDA) ─────┬── TCA9548A SDA
ESP32 GPIO 22 (SCL) ─────┼── TCA9548A SCL
                          ├── SCD30 SDA/SCL (direct on bus — I2C addr 0x61)
                          └── AS7341 SDA/SCL (direct on bus — I2C addr 0x39)
ESP32 3V3 ───────────────┬── TCA9548A VIN
                          ├── SCD30 VIN
                          └── AS7341 VIN (3.3V only — not 5V tolerant)
ESP32 GND ───────────────── all sensor GND
```

**SHT45 sensors via TCA9548A multiplexer:**

All three SHT45 sensors share I2C address 0x44. The TCA9548A puts each on its own bus channel so they can coexist:

```
TCA9548A channel 0 ── SHT45 #1 (top shelf) — SDA/SCL/3V3/GND
TCA9548A channel 1 ── SHT45 #2 (mid shelf) — SDA/SCL/3V3/GND
TCA9548A channel 2 ── SHT45 #3 (bottom)    — SDA/SCL/3V3/GND
```

TCA9548A I2C address: **0x70** (default, A0/A1/A2 all LOW).

**SCD30 placement reminder:** Mount inside the tent at mid-height on the back wall, with a small plastic baffle to deflect direct fog. Feed the I2C + power wires through a PG9 cable gland.

---

#### 1-Wire bus (DS18B20 substrate temperature probes)

```
ESP32 GPIO 4 ─────── 4.7kΩ ──── 3.3V    ← pull-up resistor
     │
     └── DS18B20 data pin (yellow wire)
             ├── Probe 1 (shelf 1)
             ├── Probe 2 (shelf 2)
             ├── Probe 3 (shelf 3)
             ├── Probe 4 (shelf 4)
             └── Probe 5 (shelf 5)

DS18B20 power (red): 3.3V
DS18B20 ground (black): GND
```

All 5 probes connect to the **same three wires** — data, power, and GND — daisy-chained along the tent. Each probe has a unique factory address so the firmware reads them individually. Label each probe with its shelf number at install time using a label maker.

**4.7kΩ pull-up:** Place this resistor between GPIO 4 and 3.3V. Only one is needed for the whole chain.

---

#### Water level sensor (4-20mA analog)

The DFRobot KIT0139 includes the sensor and a Gravity 4-20mA converter board. Wire the converter board output to the ESP32 ADC:

```
12V PSU + ──────── Sensor supply (brown wire, typically)
12V PSU − / GND ── Sensor return (blue wire, typically)
                        │
               Gravity converter board
                        │
               0–3.3V output ── ESP32 GPIO 34 (ADC input)
               GND            ── ESP32 GND
               3.3V VCC       ── ESP32 3V3
```

**Without the DFRobot converter board (DIY approach):**
```
12V+ ─── Sensor supply ─── Sensor ─── 150Ω shunt ─── 12V−/GND
                                           │
                                    ESP32 GPIO 34
                              (voltage across shunt: 0.6V–3.0V = 4–20mA)
```

Drop the sensor probe to the bottom of the 19-gallon reservoir. Route the cable through a PG9 cable gland in the enclosure.

---

### Step 4 — Failsafe Panel Switches

The failsafe allows manual load control when the ESP32 is dead, rebooting, or on a bad WiFi cycle. Wire it using a DPDT master toggle plus 4 SPST group switches.

#### How it works

The DPDT master switch sits between the ESP32 GPIO outputs and the relay IN pins:

- **AUTO position:** ESP32 GPIOs connect to relay IN pins. Normal operation.
- **MANUAL position:** ESP32 GPIOs are disconnected. The four SPST panel switches connect to the relay IN pins instead.

```
AUTO/MANUAL DPDT switch (double-pole, double-throw):

  ESP32 GPIO 16 ──[DPDT Pole A]──── Relay IN1
  ESP32 GPIO 17 ──[DPDT Pole A]──── Relay IN2    ← grouped, same throw
         │              (AUTO)            │
  GND switch 1 ──[DPDT Pole A]──── (disconnected in AUTO)
  GND switch 1 ──[DPDT Pole A]──── Relay IN1
  GND switch 1 ──[DPDT Pole A]──── Relay IN2     ← MANUAL: switch pulls to GND
```

**Simplified wiring (grouped by relay pair):**

Since each group's relays always fire together (fogger + tub fan, exhaust + intake), wire them to one switch:

```
Panel: [MASTER AUTO/MAN] [HUMIDITY] [FAE] [UVC] [LIGHTS]

MANUAL mode wiring (each SPST switch, when closed, pulls relay inputs LOW):

  HUMIDITY switch ── connects GND to Relay IN1 (fogger) + IN2 (tub fan)
  FAE switch      ── connects GND to Relay IN3 (exhaust) + IN4 (intake)
  UVC switch      ── connects GND to Relay IN5 (UVC)
  LIGHTS switch   ── connects GND to Relay IN6 (grow lights)
```

Mount all 5 switches on the enclosure lid face, clearly labeled.

---

### Step 5 — Mains Load Wiring

> ⚠️ Work in the mains zone only. Double-check the enclosure is unplugged.

Each load (fogger, fans, UVC, lights) uses the relay's **Normally Open (NO)** and **Common (COM)** contacts. The relay switches the **Hot (Live)** wire only. Neutral runs straight through to the load.

```
For each load:

  Mains L (Hot) ──── Relay COM
  Relay NO ──────── Load Live wire
  Mains N (Neutral) ─ straight through ──── Load Neutral wire
  Mains PE (Earth) ── Load ground (if load is earthed)
```

**Use Wago 221 lever connectors** for all mains connections. They are safer than bare screw terminals, can be opened for inspection, and are UL-listed. Do not use wire nuts in an enclosure.

**Wire gauge:**
- Mains in to fuse block and PSUs: 14 AWG
- Load leads (fans, fogger): 18 AWG minimum; 16 AWG preferred
- Terminate all wire ends with crimped ferrules before inserting into any terminal

**Fuse:** Fit a 5A fuse in the mains feed to the relay load terminals. Total load (2 fans ~1.5A, fogger ~0.5A, UVC ~0.5A, lights ~1A) is well under 5A.

---

### Step 6 — Final Assembly Checklist

Before first power-on:

**Low voltage side:**
- [ ] VCC/JD-VCC jumper removed from relay board
- [ ] All I2C sensors connect to 3.3V (not 5V)
- [ ] 4.7kΩ pull-up resistor on 1-Wire line
- [ ] Water level sensor output wired to GPIO 34 (input-only pin)
- [ ] All sensor GNDs tied to common GND rail
- [ ] 5V PSU − and 12V PSU − both tied to common GND

**Mains side:**
- [ ] All mains connections terminated with ferrules
- [ ] Wago connectors fully seated (lever closed)
- [ ] Fuse fitted (5A)
- [ ] No bare wire visible at any terminal
- [ ] Relay board's COM/NO contacts on mains side, IN header on low-voltage side
- [ ] Earth/PE continuity from mains to enclosure chassis

**Enclosure:**
- [ ] All cable glands tightened with cable seated (IP65 seal requires clamping the cable)
- [ ] Lid gasket seated properly
- [ ] All switch labels clearly marked

---

## GPIO Quick Reference

| GPIO | Function | Notes |
|------|----------|-------|
| 4 | 1-Wire (DS18B20 ×5) | 4.7kΩ pull-up to 3.3V |
| 16 | Relay Ch1 — Fogger | Active LOW |
| 17 | Relay Ch2 — Tub fan | Active LOW |
| 18 | Relay Ch3 — Exhaust fan | Active LOW |
| 19 | Relay Ch4 — Intake fan | Active LOW |
| 21 | I2C SDA | All I2C sensors |
| 22 | I2C SCL | All I2C sensors |
| 23 | Relay Ch5 — UVC lights | Active LOW |
| 25 | Relay Ch6 — Grow lights | Active LOW |
| 26 | Relay Ch7 — Top-off pump (opt.) | Active LOW |
| 27 | Relay Ch8 — Spare | Active LOW |
| 34 | ADC — Water level sensor | Input-only pin; 0–3.3V |

**I2C device addresses:**

| Device | Address |
|--------|---------|
| TCA9548A (I2C mux) | 0x70 |
| SHT45 ×3 (via mux ch 0/1/2) | 0x44 each |
| SCD30 (CO2) | 0x61 |
| AS7341 (light) | 0x39 |

---

## Initial Power-On Test

Do this before connecting any loads:

1. **Unplug all mains load cables** from the relay output terminals
2. Power on
3. Open a serial monitor at 115200 baud
4. Verify each I2C sensor is detected (SCD30, TCA9548A, 3× SHT45, AS7341)
5. Verify DS18B20 addresses are found on 1-Wire bus (should see 5 unique addresses)
6. Verify water level ADC reads a value (will read low if probe is in air — that's fine)
7. Manually trigger each relay from the serial console or test firmware — confirm the relay clicks and LED lights
8. Flip the MASTER switch to MANUAL, test each group switch — confirm the corresponding relay fires

Only after all sensors report and all relays confirm: reconnect loads and test under power.

---

## Sensor Placement in the Tent

| Sensor | Position | Notes |
|--------|----------|-------|
| SCD30 (CO2) | Mid-height, back wall | Small baffle to deflect direct fog; feeds I2C cable through gland |
| SHT45 #1 | Top shelf height | Clip to tent frame |
| SHT45 #2 | Middle shelf height | Clip to tent frame |
| SHT45 #3 | Bottom shelf height | Near drip tray, away from fogger pipe |
| DS18B20 ×5 | One per shelf | Rest probe against or slightly into a fruiting block |
| AS7341 | Top of tent, center | Faces down toward blocks; measures light reaching the canopy |
| Water level | Bottom of reservoir tub | Drop sensor to floor of tub; cable exits through tub lid gland |

---

## Alternatives Considered

See [diy-controller-hardware-reference.md](diy-controller-hardware-reference.md) for full comparison tables and rejection reasoning for every sensor category.

**Short version:**
- CO2: **SCD30** over SCD41 (photoacoustic is sensitive to fan vibration) and MH-Z19x (no reference channel, drifts)
- Temp/RH: **SHT45 + PTFE** over SHT3x series (SHT45 has on-chip heater that recovers from high-RH creep drift; SHT3x doesn't)
- Water level: **Submersible pressure sensor** over ultrasonic JSN-SR04T (fogger mist destroys ultrasonic readings at the water surface) and XKC-Y25 capacitive (binary only — can't give continuous level)

---

## Optional Add-on: Reservoir Top-Off Pump

The base build pre-wires relay Ch 7 and the 12V rail for this. To add the pump:

1. Mount the 12V submersible pump in a secondary reservoir (or dedicate one corner of the main tub as a clean water supply)
2. Run silicone tubing from the pump outlet to the main reservoir, terminated with a check valve
3. Connect pump leads: 12V+ → relay Ch 7 NO contact; 12V− → GND
4. Enable the water level firmware logic: set low/high thresholds for the pressure sensor, configure Ch 7 to trigger top-off cycle

The firmware monitors reservoir level continuously and runs short top-off cycles to keep the level in range — no manual filling needed.

---

## Next Steps

This guide covers hardware assembly. The firmware and web dashboard are covered in a separate guide (coming soon), based on the open-source software stack from u/mettalmag's greenhouse project, adapted for Martha tent control loops.

GitHub (software reference): https://github.com/shonomore/greenhouse-lab-demo
