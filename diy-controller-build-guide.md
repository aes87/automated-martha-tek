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

## ⚠️ CRITICAL — Hardware Corrections (Read Before Ordering Parts)

Four issues with standard component choices require correction before first power-on with mains voltage. None are expensive to fix — address them at the parts-ordering stage, not on the bench.

### C1 — Relay board 3.3V logic incompatibility

Standard PC817-based relay boards are designed for 5V logic. IN pin current-limiting resistors are 1 kΩ. At 3.3V logic: `(3.3V − 1.2V Vf) ÷ 1000Ω = 2.1 mA` through the LED. The PC817 CTR degrades badly below 5 mA — relays may fail to trigger reliably, especially when warm after a long fruiting cycle.

**Fix (choose one):**
- Replace each IN pin's 1 kΩ resistor with 470 Ω (raises LED current to ~4.5 mA, firmly in spec) — requires SMD rework
- Source a relay module explicitly labelled "3.3V compatible" / "3.3V trigger" — verify before ordering

Before connecting any mains loads: bench-test every relay from firmware and confirm it **audibly clicks** (see Step 6 — Initial Power-On Test).

### C2 — Boot-state relay glitch on GPIO 16/17 and GPIO 25/26

GPIO 16 and 17 (Ch1 Fogger, Ch2 Tub fan) are connected to UART2 on the DevKit V1 PCB. During boot and firmware flashing, UART2 TX (GPIO 17) briefly goes LOW — with active-LOW relay logic, this fires the tub fan relay at every power-on. GPIO 25/26 (DAC outputs, Ch6 Lights, Ch7 Pump) have non-deterministic state until firmware initialises them.

**The note in Step 2 stating "GPIOs default HIGH at boot" does not apply to GPIO 16, 17, 25, or 26.**

**Fixes:**
1. Add 10 kΩ pull-up resistors from each relay IN pin to the relay VCC rail — holds all relays OFF before firmware initialises GPIO direction
2. Add a 3–5 second relay-arm delay at the top of `setup()` before any relay is permitted to fire
3. **UVC (Ch5) special rule:** firmware must enforce a minimum 5-second boot delay before UVC can be armed — a boot glitch on UVC is a human safety hazard (UV exposure)
4. For v2 hardware: move Ch1/Ch2 to GPIO 32/33, which have no UART attachment and have safe boot defaults

### C3 — No overvoltage protection on GPIO 34 (ADC water level input)

The DIY 150 Ω shunt produces 3.0V at nominal 20 mA. A malfunctioning transmitter, cable fault, or wiring error can drive current above 20 mA, pushing voltage above the ESP32's **3.6V absolute maximum** on GPIO 34. The ADC pin is not self-protecting — overvoltage destroys it.

**Fix:** Add a 1 kΩ series resistor + 3.3V Zener (or SMBJ3V3A TVS diode) between the shunt output and GPIO 34. Cost: <$0.50. See the updated water level wiring in Step 3.

> **Safe beginner path:** The DFRobot KIT0139 converter board includes this protection internally. Use it and C3 does not apply to you.

### C4 — No GFCI/RCD protection (electrocution risk in wet environment)

This build operates at 80–95% RH with mains-connected loads, including a fogger that contacts standing water. **A 5A fuse does not protect against electrocution.** A fuse trips at 5,000–10,000 mA — 50–100× the lethal threshold. A GFCI/RCD trips at 5–30 mA in under 40 ms.

**In a wet environment, a GFCI/RCD is not optional.**

A GFCI outlet or 30 mA DIN-rail RCD must be the **first device** on the mains feed, before the fuse block, before anything else. This is listed as a required item in the Parts List and as a mandatory first step in the mains wiring section. This is separate from, and in addition to, having a qualified electrician do the line-side wiring.

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
| **GFCI outlet or DIN rail RCD** | **30 mA trip — REQUIRED (see C4)** | [Amazon](https://www.amazon.com/s?k=GFCI+outlet+20A) or [Automation Direct](https://www.automationdirect.com/adc/shopping/catalog/circuit_protection/ground_fault_circuit_interrupters) | **1** | **~$15–40** |
| Blade fuse block | DIN rail, 4–8 position, 2–3A fuses per load | [Amazon](https://www.amazon.com/s?k=DIN+rail+blade+fuse+block+8+position) | 1 | ~$12–15 |
| Panel switches | DPDT toggle (master) + SPST toggle ×4 | [Amazon](https://www.amazon.com/s?k=DPDT+toggle+switch+panel+mount) | 5 | ~$12 |
| Wire | 22 AWG solid/stranded (low voltage); 14 AWG (mains) | Hardware store | — | ~$10 |
| Wago 221 connectors | Push-in for mains terminals (safer than bare screws) | [Amazon](https://www.amazon.com/s?k=Wago+221+lever+connectors) | 1 pack | ~$10 |
| 2.2kΩ resistor | 1-Wire pull-up (replaces 4.7kΩ — see S3) | Electronics supplier | 1 | <$1 |
| 10kΩ resistor | Relay IN pin pull-ups (×8, one per relay channel — see C2) | Electronics supplier | 8 | <$1 |
| 150Ω resistor | Water level 4-20mA shunt (DIY path only) | Electronics supplier | 1 | <$1 |
| 1kΩ resistor | Series resistor, GPIO 34 ADC protection (see C3) | Electronics supplier | 1 | <$1 |
| 3.3V Zener or SMBJ3V3A TVS | Clamp diode, GPIO 34 ADC protection (see C3) | Electronics supplier | 1 | <$1 |
| 100kΩ resistor | GPIO 34 pull-down to GND (prevents float noise) | Electronics supplier | 1 | <$1 |
| 100nF ceramic capacitor | 1-Wire bus decoupling at pull-up junction | Electronics supplier | 1 | <$1 |
| Ferrule crimp kit | For clean screw terminal ends | [Amazon](https://www.amazon.com/s?k=ferrule+crimp+kit+22AWG) | 1 | ~$10 |
| Terminal block strip | DIN rail mounted, 10-position | [Amazon](https://www.amazon.com/s?k=DIN+rail+terminal+block+10+position) | 2 | ~$8 |
| Fuse holder + fuses | Panel mount, 5A (mains feed input) | [Amazon](https://www.amazon.com/s?k=panel+mount+fuse+holder+5A) | 1 | ~$5 |

### Misc

| Component | Notes | Est. Cost |
|-----------|-------|-----------|
| Standoffs (M3, 10mm) | For mounting ESP32 + relay board to DIN rail or base | ~$3 |
| Zip ties / velcro | Cable management | ~$3 |
| Heat shrink tubing | Insulate splices and solder joints | ~$3 |
| Label maker tape or marker | Label every wire at both ends | ~$2 |

**Base build total: ~$310–340** (includes GFCI/RCD and per-load fuse block — both required)

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
│  │  GFCI / RCD  │               │  └─────────────────────┘   │
│  │  (30 mA)     │               │                             │
│  └──────────────┘               │  ┌─────────────────────┐   │
│                                 │  │  Sensor breakouts   │   │
│  ┌──────────────┐               │  │  (TCA9548A, SCD30,  │   │
│  │  Mains fuse  │               │  │   AS7341)           │   │
│  │  Blade fuse  │               │  └─────────────────────┘   │
│  │  block (per- │               │                             │
│  │  load fuses) │               │                             │
│  │  Mains term- │               │                             │
│  │  inal blocks │               │                             │
│  └──────────────┘               │                             │
│   ← Loads exit via              │                             │
│     cable glands                │                             │
│     on bottom edge              │                             │
│─────────────────────────────────│                             │
│  HIGH VOLTAGE — DO NOT TOUCH    │  Relay board bridges both  │
│  UNLESS FULLY UNPLUGGED         │  zones (coil LV, contact HV)│
└─────────────────────────────────────────────────────────────┘
                    │ Panel face (lid):
                    │  [MASTER AUTO/MAN] [HUMIDITY] [FAE] [UVC] [LIGHTS]
```

> **GFCI/RCD position:** Mount the GFCI outlet or DIN-rail RCD as the first device the mains feed reaches inside the enclosure — before the input fuse, before the PSUs, before anything else. Every mains-connected load must be downstream of it.

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

> **3.3V compatibility:** Confirm your relay module is rated for 3.3V logic, or modify IN pin resistors per C1 above. Bench-test before connecting any loads.

#### Pull-up resistors on relay IN pins (required — see C2)

Add a 10 kΩ pull-up resistor from each relay IN pin to the relay VCC rail (3.3V). This holds every relay OFF during boot before the ESP32 initialises its GPIO pins, preventing boot-glitch relay fires:

```
Relay VCC (3.3V) ─── 10kΩ ─── Relay IN1   (repeat for IN2 through IN8)
```

Eight resistors are needed — one per channel. They can be soldered to the relay board's IN pin headers or placed on a small piece of stripboard.

#### Signal connections (ESP32 → Relay)

These run through the **failsafe DPDT switch** in normal operation (see Step 4 before soldering these).

| Relay Channel | Load | ESP32 GPIO | Boot safety note |
|---------------|------|------------|-----------------|
| IN1 | Fogger | GPIO 16 | ⚠️ GPIO 16 is UART2 RX — use pull-up |
| IN2 | Tub fan | GPIO 17 | ⚠️ GPIO 17 is UART2 TX — glitches LOW at boot; use pull-up |
| IN3 | Exhaust fan | GPIO 18 | Safe default HIGH |
| IN4 | Intake fan | GPIO 19 | Safe default HIGH |
| IN5 | UVC lights | GPIO 23 | Safe default HIGH — enforce 5s boot delay in firmware |
| IN6 | Grow lights | GPIO 25 | ⚠️ DAC1 — non-deterministic at boot; use pull-up |
| IN7 | Top-off pump (optional) | GPIO 26 | ⚠️ DAC2 — non-deterministic at boot; use pull-up |
| IN8 | Spare | GPIO 27 | Safe default HIGH |

> **Active LOW:** Relay modules trigger when the IN pin is pulled LOW. Pull-up resistors hold all IN pins HIGH (relays OFF) until the ESP32 actively drives them. Add a 3–5 second relay-arm delay in firmware `setup()` as an additional safety margin before any relay is permitted to fire.

---

### Step 3 — Sensor Wiring

#### I2C bus

All I2C sensors share two wires: SDA (GPIO 21) and SCL (GPIO 22), plus 3.3V and GND. Run a 4-wire bus from the ESP32 to each breakout board.

**Power all I2C breakouts from the ESP32's 3V3 pin, not from the 5V PSU rail.** All sensors in this build are 3.3V native. Adafruit breakouts accept 5V on VIN via an on-board regulator, but some of their I2C pull-up resistors reference VIN (5V) rather than the regulated 3.3V rail — if that is the case, SDA/SCL lines can float to 5V, which will damage the ESP32's GPIO pins and the AS7341.

> Before first power-on, check the schematic of each Adafruit breakout you are using and confirm that the on-board I2C pull-up resistors reference the 3.3V regulated rail, not VIN.
> If you use a **Waveshare AS7341** board (an alternative to the Adafruit version), add an I2C level shifter — the Waveshare board does not include on-board level shifting.

```
ESP32 GPIO 21 (SDA) ─────┬── TCA9548A SDA
ESP32 GPIO 22 (SCL) ─────┼── TCA9548A SCL
                          ├── SCD30 SDA/SCL (direct on bus — I2C addr 0x61)
                          └── AS7341 SDA/SCL (direct on bus — I2C addr 0x39)
ESP32 3V3 ───────────────┬── TCA9548A VIN  (3.3V — do not connect to 5V rail)
                          ├── SCD30 VIN
                          └── AS7341 VIN (3.3V only — not 5V tolerant)
ESP32 GND ───────────────── all sensor GND
```

> **SCD30 SEL pin:** Tie LOW (connect to GND) to select I2C mode. On the Adafruit breakout this is handled by a default solder jumper. On a bare SCD30 module, you must wire the SEL pin explicitly.

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
                        100nF ceramic
                 ┌──────┤├──────┐
ESP32 GPIO 4 ─────── 2.2kΩ ──── 3.3V    ← pull-up resistor
     │                                     (100nF bypasses cable ringing)
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

**2.2kΩ pull-up:** Place this resistor between GPIO 4 and 3.3V. Only one is needed for the whole chain. The DS18B20 datasheet recommends 2.2 kΩ for multi-sensor chains — a 4.7 kΩ pull-up gives marginal RC rise time with 5 probes on 5–8 m of cable and can cause intermittent dropout. Use 1.0 kΩ minimum if dropout occurs on long, moisture-aged cables.

**100nF decoupling capacitor:** Place a 100nF ceramic capacitor between the pull-up junction (where the 2.2kΩ connects to the data line) and GND. This suppresses cable ringing that can corrupt 1-Wire transactions.

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

**Without the DFRobot converter board (DIY approach — include ADC protection per C3):**
```
12V+ ─── Sensor supply ─── Sensor ─── 150Ω shunt ─── 12V−/GND
                                           │
                                         1kΩ            ← series protection resistor
                                           │
                                    ┌── GPIO 34         ← ADC input
                                    │
                               3.3V Zener               ← overvoltage clamp
                             (or SMBJ3V3A TVS)          (anode to GND, cathode to GPIO 34)
                                    │
                                   GND
                              also: 100kΩ pull-down
                              from GPIO 34 to GND
                              (prevents float noise when
                               sensor is disconnected)
```

Voltage across shunt at 4–20 mA: 0.6V–3.0V (nominal). The 1 kΩ series resistor + Zener clamps any fault condition to 3.3V before it reaches the GPIO pin, protecting the ESP32's 3.6V absolute maximum ADC input rating.

> **ADC calibration:** The ESP32 ADC has significant non-linearity without calibration (5–10% INL). On a 30–35 cm reservoir, this is 1.5–3.5 cm of error — enough to miss refill thresholds. Use `esp_adc_cal_characterize()` from ESP-IDF, 32+ sample rolling average, and 10% hysteresis on pump thresholds. For sub-mm accuracy, an ADS1115 16-bit I2C ADC (~$3) on the same I2C bus is a clean upgrade.

Drop the sensor probe to the bottom of the 19-gallon reservoir. Route the cable through a PG9 cable gland in the enclosure.

---

### Step 4 — Failsafe Panel Switches

The failsafe allows manual load control when the ESP32 is dead, rebooting, or on a bad WiFi cycle. Wire it using a DPDT master toggle plus 4 SPST group switches.

#### How it works — wiring topology

A single DPDT switch has two poles. It cannot individually switch all 8 relay IN signal lines. What the DPDT actually switches is the **common reference wire** that the SPST group switches use to pull relay IN pins LOW.

- **AUTO position:** The manual SPST switches' common wire is connected to VCC (3.3V). Closing a manual switch has no effect — it can only pull an IN pin toward 3.3V, which is already its pulled-up state.
- **MANUAL position:** The ESP32 is powered down or its GPIOs are in a safe state (HIGH or high-Z, held by pull-up resistors). The DPDT connects the manual SPST switches' common wire to GND. Closing a manual switch pulls the corresponding relay IN pins LOW, firing those relays.

```
                            ┌── Pole A: manual switch common wire
DPDT switch ────────────────┤   AUTO  throw → VCC (3.3V) — manual switches inactive
                            │   MANUAL throw → GND       — manual switches active
                            │
                            └── Pole B: optional LED or indicator

ESP32 GPIOs are always wired to relay IN pins.
In MANUAL, the ESP32 should be off or GPIOs held HIGH by pull-up resistors.
```

```
SPST group switches (each connected between manual common wire and relay IN pins):

  HUMIDITY switch ── common wire ──► Relay IN1 (fogger) + IN2 (tub fan)
  FAE switch      ── common wire ──► Relay IN3 (exhaust) + IN4 (intake)
  UVC switch      ── common wire ──► Relay IN5 (UVC)
  LIGHTS switch   ── common wire ──► Relay IN6 (grow lights)

In MANUAL mode (DPDT common = GND):
  Closing HUMIDITY switch pulls IN1 + IN2 LOW → fogger + fan fire
  Opening it releases → pull-up resistors hold IN1 + IN2 HIGH → relays off
```

**Why this is safe:** The 10 kΩ pull-up resistors on each IN pin (see Step 2) ensure that in the absence of an active LOW signal from either the ESP32 or a closed manual switch, all relays default to OFF. When the ESP32 is dead or rebooting, the pull-ups take over and hold everything off until you manually override.

**Important:** This failsafe design is most effective when the ESP32 is off or in reset. If the ESP32 is running and driving a relay LOW, and you flip to MANUAL mode, the ESP32 will still hold that relay on until firmware releases the GPIO. Design your firmware to respect a dedicated "MANUAL mode" GPIO input that releases all relay outputs to HIGH when detected.

Mount all 5 switches on the enclosure lid face, clearly labeled.

---

### Step 5 — Mains Load Wiring

> ⚠️ Work in the mains zone only. Double-check the enclosure is unplugged.

#### 5a — GFCI/RCD (mandatory first device — see C4)

The GFCI outlet or DIN-rail RCD must be **the first device on the mains feed** when it enters the enclosure. Wire it before the fuse holder and before the PSUs:

```
Mains in (from wall) ──── GFCI/RCD (30 mA) ──── Fuse holder (5A) ──── PSUs + relay loads
```

Test the GFCI test button after installation to confirm it trips. Do not proceed to load wiring until this is confirmed functional.

#### 5b — Per-load fusing

A single 5A fuse on the mains feed protects the input cable but will not blow for a 3A fault in one load. Add a blade fuse block with one 2–3A fuse per load:

```
After relay COM terminals:
  Relay COM (Ch1 Fogger)  ──── 2A fuse ──── Fogger Live
  Relay COM (Ch2 Tub fan) ──── 2A fuse ──── Fan Live
  Relay COM (Ch3 Exhaust) ──── 2A fuse ──── Exhaust fan Live
  Relay COM (Ch4 Intake)  ──── 2A fuse ──── Intake fan Live
  Relay COM (Ch5 UVC)     ──── 2A fuse ──── UVC Live
  Relay COM (Ch6 Lights)  ──── 3A fuse ──── Lights Live
```

Size fuses at ~125% of normal load current per channel, rounded up to the next standard fuse value.

#### 5c — Load wiring

Each load (fogger, fans, UVC, lights) uses the relay's **Normally Open (NO)** and **Common (COM)** contacts. The relay switches the **Hot (Live)** wire only. Neutral runs straight through to the load.

```
For each load:

  Mains L (Hot) ──── Relay COM ──── per-load fuse ──── Load Live wire
  Relay NO ──────── (not used for NO contact loads — see above)
  Mains N (Neutral) ─ straight through ──── Load Neutral wire
  Mains PE (Earth) ── Load ground (if load is earthed)
```

**Use Wago 221 lever connectors** for all mains connections. They are safer than bare screw terminals, can be opened for inspection, and are UL-listed. Do not use wire nuts in an enclosure.

**Wire gauge:**
- Mains in to GFCI, fuse block, and PSUs: 14 AWG
- Load leads (fans, fogger): 18 AWG minimum; 16 AWG preferred
- Terminate all wire ends with crimped ferrules before inserting into any terminal

**Main fuse:** Fit a 5A fuse in the mains feed immediately after the GFCI/RCD. This protects the input cable and wiring before the per-load fuse block.

---

### Step 6 — Final Assembly Checklist

Before first power-on:

**Low voltage side:**
- [ ] VCC/JD-VCC jumper removed from relay board
- [ ] Relay module confirmed 3.3V compatible (or IN pin resistors modified to 470Ω per C1)
- [ ] 10 kΩ pull-up resistors fitted on all 8 relay IN pins
- [ ] All I2C sensors powered from ESP32 3V3 pin (not 5V rail)
- [ ] I2C breakout pull-up resistors confirmed to reference 3.3V rail (not VIN)
- [ ] 2.2kΩ pull-up resistor on 1-Wire line (not 4.7kΩ)
- [ ] 100nF ceramic cap at 1-Wire pull-up junction to GND
- [ ] Water level sensor output: 1kΩ series + Zener/TVS clamp fitted before GPIO 34 (DIY path) or KIT0139 board used (protected path)
- [ ] 100kΩ pull-down resistor on GPIO 34 to GND
- [ ] All sensor GNDs tied to common GND rail
- [ ] 5V PSU − and 12V PSU − both tied to common GND

**Mains side:**
- [ ] GFCI/RCD fitted and tested (Test button trips; Reset button restores) — **first device on mains feed**
- [ ] All mains connections terminated with ferrules
- [ ] Wago connectors fully seated (lever closed)
- [ ] Main fuse fitted (5A) downstream of GFCI/RCD
- [ ] Per-load blade fuse block installed with correct fuse ratings per channel
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
| 4 | 1-Wire (DS18B20 ×5) | 2.2kΩ pull-up to 3.3V; 100nF decoupling cap at junction |
| 16 | Relay Ch1 — Fogger | Active LOW; UART2 RX — add 10kΩ pull-up to relay VCC |
| 17 | Relay Ch2 — Tub fan | Active LOW; UART2 TX — glitches LOW at boot; add 10kΩ pull-up |
| 18 | Relay Ch3 — Exhaust fan | Active LOW; safe default HIGH |
| 19 | Relay Ch4 — Intake fan | Active LOW; safe default HIGH |
| 21 | I2C SDA | All I2C sensors |
| 22 | I2C SCL | All I2C sensors |
| 23 | Relay Ch5 — UVC lights | Active LOW; enforce 5s boot delay before arming |
| 25 | Relay Ch6 — Grow lights | Active LOW; DAC1 output — non-deterministic at boot; add 10kΩ pull-up |
| 26 | Relay Ch7 — Top-off pump (opt.) | Active LOW; DAC2 output — non-deterministic at boot; add 10kΩ pull-up |
| 27 | Relay Ch8 — Spare | Active LOW; safe default HIGH |
| 34 | ADC — Water level sensor | Input-only; 1kΩ series + 3.3V Zener/TVS protection required; 100kΩ pull-down to GND |

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
4. **Relay boot-state check:** Immediately after power-on, before firmware runs, verify that no relay clicks. Any relay that fires during boot indicates a pull-up or GPIO assignment problem — do not proceed until resolved (see C2)
5. **Relay compatibility test:** Manually trigger each relay from the serial console or test firmware. Confirm every relay **audibly clicks** and its indicator LED illuminates. If any relay fails to click (coil hums or no response), you have a 3.3V logic incompatibility — see C1 before proceeding
6. Verify each I2C sensor is detected (SCD30, TCA9548A, 3× SHT45, AS7341)
7. Verify DS18B20 addresses are found on 1-Wire bus (should see 5 unique addresses)
8. Verify water level ADC reads a value in a reasonable range; confirm no ADC pin overvoltage (measure voltage at GPIO 34 pin — should be ≤3.3V)
9. Flip the MASTER switch to MANUAL, test each group switch — confirm the corresponding relay fires, and confirm other relays do not fire
10. **GFCI test:** With a GFCI outlet, press the Test button — power should cut. Press Reset to restore. Do not skip this step.

Only after all sensors report, all relays confirm, and GFCI is tested: reconnect loads and test under power.

> **Firmware safety requirement before live use:** Ensure your `setup()` function includes a 3–5 second delay before enabling any relay control, and a minimum 5-second guard on the UVC channel before it can first fire.

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

## Platform Notes and Known Limitations

### ESP32 DevKit V1 — CH340 moisture risk

The DevKit V1 carries the USB-to-UART bridge chip (CH340 or CP2102) and micro-USB port on the carrier PCB, exposed inside the enclosure. In a sealed box with 80–95% RH ambient, these metal contacts and chips oxidize over months, potentially corrupting USB connections. The WROOM-32 module itself is more durable.

**Mitigations:**
- Flash firmware and test everything on the bench before sealing the enclosure
- Set up OTA (over-the-air) firmware updates for all subsequent flashing — do not repeatedly open the enclosure to plug in USB
- If building fresh today, consider the **ESP32-S3** as the starting platform (see below)

### ESP32-S3 as a better starting point for new builds

The ESP32-S3 resolves several issues in this guide:
- **Native USB** — no CH340/CP2102 chip; USB-C directly on the chip. No oxidation-prone UART bridge
- **No GPIO 16/17 UART2 issue** — GPIO mapping is different; UART pins do not overlap with relay assignments in the same way
- **Better ADC calibration support** — ESP-IDF ADC calibration API has better characterisation support on the S3
- Same price as the DevKit V1 (~$10)

If you are starting a fresh build, the ESP32-S3 DevKitC-1 is the recommended platform. GPIO mapping in this guide will need minor adjustment.

### WiFi/OTA security note

The web dashboard must **not** be exposed outside your local network. A relay controller accessible from the internet is a fire and safety risk.
- Use WPA2 on your local WiFi; do not set up port forwarding to this device
- If implementing OTA firmware updates, use HTTPS/TLS — do not accept unsigned firmware over plaintext HTTP

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
