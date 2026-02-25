# DIY Controller — Hardware Specification

> **Reference build:** u/mettalmag, r/MushroomGrowers (Jan 2025) — 200sqm commercial greenhouse
> Original post: https://www.reddit.com/r/MushroomGrowers/comments/1rao1ms/
> GitHub (full software stack): https://github.com/shonomore/greenhouse-lab-demo
>
> This document adapts mettalmag's hardware architecture for a home Martha tent fruiting chamber. The goal is a highly-featured, accurate, data-rich controller — not a minimum viable one.

---

## Architecture Overview

**Brains:** ESP32 DevKit V1 — WiFi, dual-core, ample GPIO, native I2C/UART/1-Wire support.

**Switching:** 8-channel opto-isolated relay module — controls all loads (fans, fogger, UVC, lights). Opto-isolation is not optional; it protects the ESP32 from voltage spikes when inductive loads (fans) switch off.

**Sensors:** Purpose-selected for a 80–95% RH continuous environment. Cheap sensors fail here — mettalmag's original HTU21D was destroyed by humidity. Every sensor choice below is rated or hardware-protected for this environment.

**Enclosure:** IP65 sealed box. In a humid environment this is the difference between a controller that lasts years and one that corrodes in months.

**Failsafe:** Physical manual override switches on the panel face — independent of the ESP32. If WiFi drops, the server crashes, or a part fries, loads can still be controlled manually.

---

## Relay 3.3V Logic Compatibility Analysis

**This is the most common hidden failure mode in ESP32 relay builds.** Most 8-channel relay modules sold on Amazon/AliExpress use PC817 optocouplers with 1 kΩ IN pin resistors — designed for 5V logic. When driven at 3.3V:

```
LED forward current at 3.3V:
  I = (Vcc − Vf) / R
  I = (3.3V − 1.2V) / 1000Ω
  I = 2.1 mA

Minimum reliable CTR current for PC817: ~5 mA
Result at 3.3V: 2.1 mA — well below reliable activation threshold
```

At 2.1 mA, the PC817's current transfer ratio is near its lower bound. At room temperature this may work — but relay modules warm up during operation. As the PCB heats, LED Vf increases and drive current drops further. **A relay that clicks reliably during a 2-minute bench test may fail intermittently during a 6-hour fruiting cycle.**

### Fix Option A — Resistor modification

Replace the 1 kΩ IN pin resistors with 470 Ω:
```
  I = (3.3V − 1.2V) / 470Ω
  I = 4.5 mA  ← within PC817 reliable range (5 mA nominal, 1 mA absolute minimum)
```
This requires SMD rework on the relay board. The resistors are small but accessible.

### Fix Option B — Source a 3.3V-compatible module

Several relay modules are explicitly designed for 3.3V microcontroller logic. Search terms: "3.3V relay module", "3.3V trigger relay", "ESP32 relay module". Verify the IN pin drive circuit before ordering — some listings claiming "3.3V compatible" still use 1 kΩ resistors.

### Fix Option C — Use a buffer/level shifter

Insert a 74AHCT125 or similar 3.3V→5V level shifter between the ESP32 GPIOs and the relay IN pins. Adds complexity but is the cleanest electrical solution and removes any marginal-current concerns permanently.

### Bench verification (required regardless of fix chosen)

Before connecting any mains loads, trigger each relay from firmware and confirm every relay **audibly clicks**. A relay that fails to fully actuate will not trip its mechanism; you can hear the difference between a proper mechanical click and a half-actuated coil hum.

---

## Microcontroller Platform Comparison

| Feature | ESP32 DevKit V1 | ESP32-S3 DevKitC-1 | Raspberry Pi Pico W |
|---------|----------------|-------------------|---------------------|
| Core | Xtensa dual-core @ 240 MHz | Xtensa dual-core @ 240 MHz | Cortex-M0+ @ 133 MHz |
| WiFi | 802.11 b/g/n | 802.11 b/g/n | 802.11 b/g/n |
| USB | CH340/CP2102 bridge chip | Native USB (no bridge) | Native USB (no bridge) |
| GPIO count | 34 usable | 45 usable | 26 usable |
| ADC | 12-bit, 2 channels, significant INL | 12-bit, better calibration support | 12-bit, good linearity |
| I2C | Hardware, GPIO 21/22 | Hardware, remappable | Hardware, GPIO 4/5 |
| 1-Wire | Any GPIO | Any GPIO | Any GPIO |
| Boot GPIO quirks | GPIO 16/17 = UART2; GPIO 25/26 = DAC | No relay-relevant UART overlap | No UART boot issues |
| Humidity durability | CH340 oxidation risk at 80–95% RH | Better (no bridge chip) | Better (no bridge chip) |
| Ecosystem / libraries | Mature, large | Growing, mostly compatible | MicroPython/C SDK |
| Price | ~$10 | ~$10–12 | ~$6 |
| **Verdict** | Works; has known quirks | **Recommended for new builds** | Viable; fewer libraries |

### ESP32-S3 for new builds

The ESP32-S3 resolves the two main DevKit V1 hardware issues (CH340 moisture risk and UART2 boot glitch on GPIO 16/17) with no price premium. GPIO mapping in the build guide will need minor adjustment — the S3 has a different pinout. The I2C, 1-Wire, and relay channel assignments can be remapped to any available GPIOs.

### Raspberry Pi Pico W — viable but non-standard

The Pico W is a capable platform with native USB and good WiFi. It lacks the mature ESP32 ecosystem and the wide range of Arduino/ESP-IDF relay/sensor libraries. Most available Martha controller firmware is written for ESP32. Reasonable choice for experienced MicroPython users starting from scratch; not recommended if you plan to adapt existing ESP32 firmware.

---

## PSU Selection

### Current recommendation (updated)

Cheap "Amazon 5V 2A" supplies have poor transient response. The four fog/FAE relay coils can all fire simultaneously during FAE + humidity co-trigger, causing a ~320 mA step load on the 5V rail. A supply with poor transient response will droop and potentially brownout the ESP32 or cause sensor glitches.

**Recommended: Use a Meanwell or Mornsun DIN-rail PSU, minimum 5V 3A.**

| Spec | "Amazon Generic" | Meanwell HDR-30-5 | Mornsun LMFT30-23B05 |
|------|-----------------|-------------------|-----------------------|
| Price | ~$8–10 | ~$18–22 | ~$16–20 |
| Load regulation | Unspecified | ±0.5% | ±1.0% |
| Transient response | Unspecified | Good (industrial grade) | Good |
| Ripple | Unspecified | 80 mV | 100 mV |
| MTBF | Unknown | 300,000+ hours | 200,000+ hours |
| DIN rail | Sometimes | Yes (native) | Yes (native) |

The cost difference is $8–12 for the 5V supply and a similar premium for the 12V supply. Over a grow controller that runs 24/7 for years, the reliability difference is significant.

**5V supply: 3A minimum** — provides headroom for the relay coil surge (4 coils × ~80 mA each = 320 mA peak) plus ESP32 (~250 mA WiFi active) plus sensors (~150 mA total).
**12V supply: 1A** remains adequate for the water level sensor and optional top-off pump.

---

## Sensors

### CO2 — Sensirion SCD30
**Adafruit #4867 — ~$59**

The SCD30 is the clear choice for accuracy and long-term stability in a humid environment.

| Spec | Value |
|------|-------|
| Technology | Dual-channel NDIR |
| Accuracy | ±(30 ppm + 3% of reading) |
| Range | 400–10,000 ppm |
| Max RH | 95% non-condensing |
| Interface | I2C / Modbus |
| Supply | 3.3V or 5V |

**Why not the alternatives:**
- **MH-Z19B/C**: No reference channel — real-world drift over 6–12 months is significantly worse. Works, but not for this build.
- **SCD41**: Photoacoustic — measurably sensitive to air currents and fan vibration (both present in a Martha). Unit-to-unit spread in practice is larger than the spec suggests.
- **SCD40**: 400–2,000 ppm ceiling is a disqualifier. Oyster/shiitake can exceed this during pinning without FAE.

**Placement:** Mount inside the tent at mid-height on the back wall — away from the fogger inlet (top-right) and the exhaust outlet (bottom-left). Attach a small plastic or 3D-printed baffle in front of the sensor face to deflect direct mist while allowing air to circulate freely around it. Mid-height keeps it out of both the warm dry stratification layer at the top and the cool fog layer at the bottom, giving a representative reading of the column the blocks actually live in. All CO2 sensors are rated 95% RH non-condensing — condensation on the sensor face will temporarily corrupt readings.

---

### Temp + Humidity — Sensirion SHT45 + PTFE membrane
**Adafruit #6174 — ~$13.50 each**

The SHT45 is Sensirion's top-tier humidity/temperature IC. The PTFE filter membrane version is the right choice for a fogger environment.

| Spec | Value |
|------|-------|
| RH accuracy | ±1.0% RH typical (full 0–100% range) |
| Temp accuracy | ±0.1°C |
| Interface | I2C |
| Supply | 3.3V native |
| PTFE filter | Blocks liquid droplets and particles ≥200nm; passes vapor |

**Critical advantage over SHT3x series:** The SHT45 includes an on-chip programmable heater. In a chamber running 85–95% RH continuously, polymer humidity sensors develop a +3% RH creep offset after extended high-RH exposure. A brief heater pulse (1 second, once per hour) burns this off and restores calibrated accuracy. The SHT30/31/35 have no heater — they drift and stay drifted.

**Deploy 3 sensors — top / mid / low:**
Temperature and humidity in a Martha tent are not uniform. Fog enters at the top-right; CO2 exits at the bottom-left. The top shelf is warmer and drier; the bottom is cooler and wetter. A single sensor gives you one point. Three sensors across shelf heights give you a full gradient map and make stratification problems visible. Oysters on the top shelf experiencing 5% less RH than your dashboard shows is a common invisible problem.

**I2C note:** All SHT4x sensors share the same I2C address. Use a **TCA9548A I2C multiplexer** (Adafruit #2717, ~$7) to put each sensor on its own channel.

---

### Water Level — Submersible Hydrostatic Pressure Sensor
**AliExpress (0–0.5m or 0–1m range) — ~$15–20**
**or DFRobot KIT0139 with converter board — ~$44**

Continuous, accurate water level reading for the 19-gallon fogger reservoir.

| Spec | Value |
|------|-------|
| Output | 4–20 mA (2-wire) |
| Accuracy | 0.5% FS (~2.5 mm over 0.5 m range) |
| IP rating | IP68 |
| No moving parts | Yes |

**Why not ultrasonic (JSN-SR04T):** The fogger creates a dense mist layer on the water surface that scatters ultrasonic pulses unpredictably. The JSN-SR04T has a 25 cm blind zone — in a tub only 30–35 cm deep, that leaves very little usable range. This is the wrong tool for this application.

**Why not capacitive (XKC-Y25):** All variants of the XKC-Y25 output a binary on/off signal. It is a point-level switch, not a level transmitter. Cannot give continuous readings.

**ESP32 integration:** Wire a 150Ω shunt resistor across the signal line to convert 4–20 mA into 0.6–3.0V for the ESP32 ADC (within the 3.3V ADC range). The DFRobot KIT0139 kit includes a matched converter board if you prefer not to calculate the resistor network yourself.

**Power note:** Industrial 4–20 mA sensors typically require 12–24VDC supply. Options:
- Use a 12V supply rail in the enclosure alongside the 5V rail (recommended — clean solution)
- Use a 5V-output variant (available on AliExpress) with direct 0.5–4.5V ratiometric output, eliminating the current loop entirely. Easier for a first build.

---

### Substrate Temperature — DS18B20 Waterproof Probes
**Adafruit #381 — ~$10 each**

One probe per shelf — 5 probes total for a 5-shelf Martha. Rested against or inserted into a fruiting block on each shelf.

| Spec | Value |
|------|-------|
| Accuracy | ±0.5°C (-10 to +85°C) |
| Interface | 1-Wire (single ESP32 GPIO) |
| Addressing | Unique 64-bit ROM per sensor — daisy-chainable |
| Form | Stainless steel waterproof probe, 1m cable |

Actively fruiting and colonizing mycelium generates metabolic heat. Substrate temperature runs warmer than air temperature, especially during heavy pinning. Monitoring substrate temp per shelf lets you detect overheating blocks before it stresses the crop — something air sensors completely miss.

**1-Wire chains all 5 sensors on one GPIO pin.** Each DS18B20 has a factory-burned unique address, so all 5 probes are individually addressable on a single wire.

---

### Light / Spectrum — AS7341 11-Channel Spectral Sensor
**Adafruit #4698 or Waveshare — ~$10–15**

| Spec | Value |
|------|-------|
| Channels | 8 visible (415–680 nm) + NIR + flicker detection |
| Interface | I2C |
| Supply | 3.3V |

Mushrooms don't photosynthesize but use light — especially blue spectrum (~450–470 nm) — as a directional and developmental cue. The AS7341 tells you the actual spectral composition your LED strip is delivering, not just lux. It will also confirm your timer is working and detect if your LED driver is dimming over time.

An LDR (photoresistor) tells you only total light level. The AS7341 tells you whether the blue spectrum is present, how much, and whether it's stable. For a simple lux/on-off check, the AS7341 is overkill. For a data-rich build, it's a cheap and meaningful upgrade.

**Note on 3.3V:** The AS7341 is not 5V-tolerant on I2C. Use the 3.3V I2C bus on the ESP32 — which is correct anyway for all sensors in this build.

---

## Derived Metrics (no additional hardware)

**VPD (Vapor Pressure Deficit)** is calculated from temp and RH already being measured:

```
VPD = (1 − RH/100) × 0.6108 × exp(17.27 × T / (T + 237.3))
```

Target for gourmet fruiting: **0.3–0.8 kPa**. High VPD causes cracking and aborted pins; low VPD (near-saturated air) invites bacterial blotch and slow development. With three SHT45 sensors, you get a VPD reading per shelf height — a meaningful data point that most Martha setups completely lack.

**Dew point** is similarly derived. Useful for understanding condensation risk inside the enclosure.

---

## Relay Channel Allocation

| Ch | Load | Trigger | Notes |
|----|------|---------|-------|
| 1 | Fogger | RH controller (RH < 85%) | |
| 2 | Tub fan (fog pusher) | Same as fogger — always together | |
| 3 | Exhaust fan (FAE out) | CO2 controller (CO2 > 950 ppm) | |
| 4 | Intake fan (FAE in) | Same as exhaust — always together | |
| 5 | UVC reservoir lights | Timer (1 hr ON / 4 hr OFF) | |
| 6 | LED grow lights | Timer or manual | |
| 7 | Top-off pump | Water level (optional add-on — see below) | Wired at build time; pump not installed in base build |
| 8 | Spare | — | |

---

## Physical Failsafe Panel

All loads covered — operable without the ESP32 for maintenance or failure recovery. Loads are grouped by natural pairs (fogger + tub fan always run together; intake + exhaust always run together), so 5 switches covers all 4 groups independently.

| Switch | Controls | Type |
|--------|----------|------|
| AUTO / MANUAL master | Cuts the relay board signal lines; enables the manual switches below | DPDT toggle |
| Humidity ON/OFF | Fogger + tub fan together (Ch 1 + 2) | SPST toggle |
| FAE ON/OFF | Exhaust fan + intake fan together (Ch 3 + 4) | SPST toggle |
| UVC ON/OFF | Reservoir UVC lights (Ch 5) | SPST toggle |
| Lights ON/OFF | LED grow lights (Ch 6) | SPST toggle |

**Wiring topology:** The AUTO/MANUAL master switch intercepts the low-voltage signal lines between the ESP32 and the relay board input pins. In AUTO, the ESP32 drives the relays normally. In MANUAL, the ESP32 is disconnected from the relay inputs and the four group switches connect directly instead. This means the relays themselves (and the loads) work identically in both modes — the only thing that changes is what's driving the relay coil inputs.

If WiFi drops, firmware crashes, or a relay board fries — flip to MANUAL and keep the grow running.

---

## Full BOM

| Component | Part | Source | Qty | Est. Cost |
|-----------|------|--------|-----|-----------|
| Microcontroller | ESP32 DevKit V1 | Amazon / AliExpress | 1 | ~$10 |
| Relay module | 8-ch 5V opto-isolated relay | Amazon / AliExpress | 1 | ~$10 |
| CO2 sensor | Sensirion SCD30 | Adafruit #4867 | 1 | $58.95 |
| Temp/RH sensor | Sensirion SHT45 + PTFE | Adafruit #6174 | 3 | $40.50 |
| I2C multiplexer | TCA9548A | Adafruit #2717 | 1 | $6.95 |
| Water level sensor | Submersible 0–0.5m pressure sensor | AliExpress | 1 | ~$15–20 |
| Substrate temp | DS18B20 waterproof probe | Adafruit #381 | 5 | ~$50 |
| Light sensor | AS7341 spectral | Adafruit #4698 | 1 | ~$12 |
| Enclosure | IP65 sealed box (min. 200×120×75mm) | Amazon / AliExpress | 1 | ~$15–20 |
| PSU — logic | 5V **3A** DIN rail (Meanwell HDR-30-5 or Mornsun equivalent) | Mouser / Digi-Key | 1 | ~$18–22 |
| PSU — peripherals | 12V 1A DIN rail (Meanwell HDR-15-12 or equivalent) | Mouser / Digi-Key | 1 | ~$15–18 |
| Misc | Fuses, resistors, terminal blocks, velcro, wire | — | — | ~$15 |
| Panel switches | DPDT master (1) + SPST group switches (4) | Amazon / AliExpress | 5 | ~$12 |
| **Base build total** | | | | **~$270–285** |

### Optional Add-on: Reservoir Top-Off Pump

> **Not installed in the base build.** Relay Ch 7 and the 12V rail are pre-wired to support this. Adding the pump at any time requires connecting it to Ch 7 and enabling the firmware logic — no enclosure rewiring needed.

| Component | Part | Source | Est. Cost |
|-----------|------|--------|-----------|
| Submersible mini pump | 12V DC submersible pump, 3–5 L/min | Amazon / AliExpress | ~$8–12 |
| Tubing | 3/8" or 1/2" food-safe silicone tubing | Amazon | ~$5 |
| Float valve or check valve | Prevents backflow | Amazon | ~$5 |
| **Add-on total** | | | **~$18–22** |

The water level sensor's continuous reading drives the pump logic: firmware triggers Ch 7 when level drops below a set threshold and cuts it when full, keeping the reservoir topped off automatically.

---

## Alternatives Considered

A record of what was evaluated and rejected for each sensor category, with reasoning. Intended as source material for a future build guide so others can understand the decision rationale.

---

### CO2 Sensor Alternatives

| Sensor | Technology | Accuracy | Price | Verdict |
|--------|-----------|----------|-------|---------|
| **SCD30** ✓ | Dual-channel NDIR | ±(30 ppm + 3%) | ~$59 | **Selected** |
| SCD41 | Photoacoustic | ±(40 ppm + 5%) | ~$45 | Close runner-up — rejected |
| SCD40 | Photoacoustic | ±(50 ppm + 5%) | ~$30 | Rejected — ceiling too low |
| MH-Z19C | Single-channel NDIR | ±(50 ppm + 5%) | ~$15–25 | Rejected — budget option only |
| MH-Z19B | Single-channel NDIR | ±(50 ppm + 3%) | ~$15–20 | Rejected — superseded by 19C |

**SCD41 — why rejected despite being close:** Photoacoustic sensors measure CO2 by detecting the pressure wave produced when the gas absorbs light from a pulsed laser. This makes them inherently sensitive to ambient air currents and physical vibration — both of which are continuous in a Martha tent with fans running. Real-world forum reports (Adafruit forums, CleanAir community) show larger unit-to-unit spread than the spec sheet suggests. The SCD30's dual-channel NDIR reference design eliminates this class of noise at the hardware level. For a Martha where fans cycle on/off repeatedly, the SCD30 is the more robust choice.

**SCD40 — why rejected:** 400–2,000 ppm measurement ceiling. Oyster and shiitake mushrooms can push well above 2,000 ppm in a sealed tent without FAE. Ceiling disqualifies it.

**MH-Z19B/C — why rejected:** No reference channel. Long-term drift is significantly worse than Sensirion NDIR designs — real-world comparisons show the MH-Z19 variants diverging noticeably after 6–12 months without forced recalibration. The MH-Z19C is a perfectly serviceable budget sensor for a first build or a timer-based approach, but not for an accurate long-term controller.

---

### Temp + Humidity Sensor Alternatives

| Sensor | RH Accuracy | Temp Accuracy | Heater | Humidity-protected variant | Price | Verdict |
|--------|------------|--------------|--------|--------------------------|-------|---------|
| **SHT45 + PTFE** ✓ | ±1.0% RH | ±0.1°C | Yes | Yes (PTFE membrane) | ~$13.50 | **Selected** |
| SHT35-DIS-F | ±1.5% RH | ±0.1°C | No | Yes (PTFE filter variant) | ~$8–10 (bare die) | Rejected — no heater |
| SHT31-DIS-F | ±2.0% RH | ±0.3°C | No | Yes (PTFE filter variant) | ~$14 (Adafruit breakout) | Rejected — no heater, lower accuracy |
| SHT30 (Taidacent Modbus) | ±2.0% RH | ±0.3°C | No | Yes (probe housing) | ~$20–30 | Rejected — RS485 complexity, no heater |
| HTU21D | ±2.0% RH | ±0.3°C | No | No | ~$5 | Rejected — mettalmag's unit failed in 90%+ RH |

**SHT35 — why rejected despite good specs:** The SHT35 has excellent accuracy (±1.5% RH, ±0.1°C) and is available in a PTFE filter variant. The reason it loses to the SHT45 is the absence of an on-chip heater. In an environment running 85–95% RH continuously, all polymer capacitive humidity sensors develop a positive drift offset (~+3% RH) after extended high-RH exposure — known as "humidity creep." Without the heater, the sensor drifts and stays drifted. The SHT45's heater pulse (fire for 1 second, once per hour) burns the creep off and restores calibrated accuracy. For this use case the heater is not a nice-to-have; it is the deciding feature.

**SHT30/31 — why rejected:** Same heater issue as SHT35, plus meaningfully lower accuracy (±2% RH vs ±1%). Worth noting for a budget build: the SHT31-D breakout (Adafruit #2857, ~$14) is widely available and a reasonable entry-level choice. Just accept the drift.

**Taidacent SHT30 Modbus — why rejected:** The industrial probe form factor and RS485 Modbus output are useful for long cable runs in a commercial rack. For a home Martha tent controller on I2C, it adds a hardware RS485 transceiver with no accuracy gain (still an SHT30 die: ±2% RH, no heater). Not the right tradeoff here.

**HTU21D — why rejected:** Direct lesson from mettalmag: destroyed by continuous 90%+ RH. Do not use.

---

### Water Level Sensor Alternatives

| Method | Continuous output | Accuracy | Contact with water | Recommended for this use | Price |
|--------|-----------------|----------|-------------------|--------------------------|-------|
| **Submersible pressure sensor** ✓ | Yes | 0.5% FS (~2–3 mm) | Yes (submerged) | **Yes — selected** | ~$15–20 |
| JSN-SR04T (ultrasonic) | Yes | ~1–3 mm (ideal) | No | No | ~$8–12 |
| XKC-Y25 (capacitive non-contact) | No — binary only | Point detection | No | No | ~$8–12 |
| Float switches (multi-height) | No — discrete steps | 3–5 level steps | Yes | Acceptable fallback | ~$5–15 total |
| 4–20 mA + DFRobot KIT0139 | Yes | 0.5% FS | Yes (submerged) | Yes — easier integration | ~$44 |

**JSN-SR04T — why rejected despite being the obvious first choice:** The fogger creates a dense mist layer on the water surface that scatters ultrasonic pulses unpredictably. Typical reported results in fogger/humidifier applications show noisy, unstable readings when the fogger is running — which is exactly when accurate level monitoring matters. Additionally, the JSN-SR04T has a 25 cm minimum blind zone from the sensor face. In a standard 19-gallon tub (interior height typically 30–35 cm), mounting the sensor on the lid places it within or close to the blind zone at low water levels. These two problems together disqualify it for this specific application, even though it works well in clear, still water.

**XKC-Y25 — why rejected:** Commonly suggested as a non-contact option, but every variant of the XKC-Y25 family outputs a binary digital signal (NPN or PNP open-collector, or RS485 on/off). It is a point-level switch, not a level transmitter. There is no continuous analog output. It cannot tell you how full the tank is — only whether water is present at a single point. Disqualified.

**Float switches at multiple heights — acceptable fallback:** Three float switches (e.g. at 25%, 50%, 75% full) give a quasi-continuous level with discrete steps. Reliable, cheap, no signal conditioning needed. The downside is coarse resolution — you can predict "needs refilling soon" but not "X litres remaining." For a budget build this is a reasonable approach. For a data-rich build the pressure sensor wins.

**DFRobot KIT0139 — same sensor, easier integration:** This is the same class of submersible hydrostatic pressure sensor but sold as a kit with a matched Gravity 4-20mA-to-voltage converter board pre-configured for Arduino/ESP32. Higher price (~$44 vs ~$15–20) for the convenience of the matched converter. Good option if you want to avoid calculating the shunt resistor network.

---

### Water Level Sensor — Power Supply Note

Industrial 4–20 mA pressure sensors typically require 12–24VDC excitation. Two approaches:

- **12V supply rail in enclosure (recommended):** Add a 12V 1A supply alongside the 5V rail. Gives a clean solution and leaves headroom for other 12V loads (e.g. a top-off pump on Ch 7).
- **5V ratiometric variant:** Some AliExpress sensors output 0.5–4.5V ratiometric on 5V supply, eliminating the current loop entirely. Simpler first-build option. Quality is more variable — check reviews carefully.

---

### Additional Sensors — Alternatives Considered

**Substrate temperature — DS18B20 chosen over (5 probes, one per shelf):**
- Thermocouple + MAX31855 amplifier: Higher temperature range (not needed), more complex wiring, one GPIO per sensor. DS18B20's 1-Wire daisy-chain is clearly superior for multiple sensors at this temperature range.
- Analog NTC thermistor: Requires per-sensor ADC channel and individual calibration. Practical for one or two sensors, doesn't scale to 4 probes cleanly.

**Light sensor — AS7341 chosen over:**
- LDR / photoresistor: Gives only aggregate lux. No spectral information — cannot tell you whether blue spectrum is present or has shifted.
- BH1750 / TSL2591: Broadband lux sensors. Better than an LDR, but still no spectral resolution. Can't distinguish a blue-heavy LED from a warm white one.
- Full PAR meter: Accurate PPFD measurement, but purpose-built PAR meters are expensive (~$100–300+) and not designed for ESP32 integration. The AS7341 can approximate PAR via a published linear regression model at a fraction of the cost.

**Sensors explicitly not included:**
- **BME280 (barometric pressure inside tent):** The SHT45 already provides temp + RH with better accuracy. Barometric pressure is not an actionable variable in a Martha tent — only relevant for outdoor weather prediction (mettalmag's use case). Not included.
- **SEN55 (Sensirion particulate matter + VOC + CO2 combo):** Adds PM1.0/PM2.5/PM4.0/PM10 and NOx/VOC indices. Particulate sensing has no actionable application in fruiting chamber management. Adds ~$50+ of cost for irrelevant data. Not included.
- **Soil/substrate moisture (capacitive or resistance):** Consumer capacitive moisture sensors are designed for mineral soil. They read unreliably in sterilized grain or sawdust substrate. Not included.
- **pH sensors:** Fragile in substrate, require frequent calibration, marginal actionable data compared to preparing substrate correctly. Not included.

---

## Decisions Log

| Item | Decision | Rationale |
|------|----------|-----------|
| Water level sensor power | 12V supply rail | Cleaner architecture; supports top-off pump add-on on same rail |
| CO2 sensor placement | Mid-height, back wall, with mist baffle | Representative of column conditions; protected from fog and condensation |
| DS18B20 probe count | 5 (one per shelf) | 5-shelf Martha; full per-shelf substrate temp coverage from day one |
| Failsafe switch scope | All 4 load groups | Fogger+fan / FAE / UVC / Lights; grouped by natural pairs |
| Failsafe switch topology | DPDT master + 4× SPST group switches | Master disconnects ESP32 from relay inputs; group switches drive relays directly in MANUAL mode |
| Top-off pump | Optional add-on, Ch 7 pre-wired | 12V rail and relay channel reserved; pump not installed in base build |
