# Chapter 01 — Parts & Tools

Order everything before starting Chapter 02. The build has no complex dependencies
between suppliers — you can order in parallel.

**Estimated total: ~$310–340** (base build, no optional pump add-on).

---

## Parts by Supplier

### Adafruit — Sensors

Order from [adafruit.com](https://www.adafruit.com). These are the sensors that
tolerate a fruiting chamber environment. Do not substitute without reading the
hardware reference.

| # | Part | Adafruit SKU | Qty | ~Price |
|---|------|-------------|-----|--------|
| 1 | Sensirion SCD30 CO2/Temp/RH sensor | [#4867](https://www.adafruit.com/product/4867) | 1 | $58.95 |
| 2 | Sensirion SHT45 Temp/RH sensor (PTFE membrane) | [#6174](https://www.adafruit.com/product/6174) | 3 | $40.50 |
| 3 | TCA9548A I2C multiplexer breakout | [#2717](https://www.adafruit.com/product/2717) | 1 | $6.95 |
| 4 | DS18B20 waterproof temp probe, 1m cable | [#381](https://www.adafruit.com/product/381) | 5 | ~$50 |
| 5 | AS7341 11-channel spectral light sensor | [#4698](https://www.adafruit.com/product/4698) | 1 | ~$12 |

> **[?] PTFE membrane:** PTFE (polytetrafluoroethylene) is a porous material that
> lets water vapour through but blocks liquid droplets. The SHT45 needs this because
> it will be inside a tent running 85–95% relative humidity — liquid droplets landing
> on the sensor would corrupt readings and eventually damage it.

> **[?] I2C multiplexer:** All three SHT45 sensors share the same I2C address
> (0x44), so you can't put them directly on the same bus — they'd all respond at
> once. The TCA9548A is a switch that lets you talk to one sensor at a time by
> selecting which of its 8 channels is active. Think of it as a multi-port USB hub
> for I2C devices that share an address.

---

### Mouser / Digi-Key — Power Supplies

Source industrial-grade DIN-rail power supplies here. Do not substitute with Amazon
generic PSUs — see the hardware reference for the reason (poor transient response
when multiple relay coils fire simultaneously).

| # | Part | Source | Qty | ~Price |
|---|------|--------|-----|--------|
| 6 | Meanwell HDR-30-5 (5V 3A DIN rail PSU) | [Mouser](https://www.mouser.com/ProductDetail/Meanwell/HDR-30-5) | 1 | ~$18–22 |
| 7 | Meanwell HDR-15-12 (12V 1A DIN rail PSU) | [Mouser](https://www.mouser.com/ProductDetail/Meanwell/HDR-15-12) | 1 | ~$15–18 |

**Alternatives:** Mornsun LMFT30-23B05 (5V) and LMFT15-23B12 (12V) are equivalent
DIN-rail units. Both are fine.

> **[?] DIN rail PSU:** A power supply designed to snap onto a standard 35mm DIN
> rail — the metal channel you'll mount inside the enclosure. Compared to a
> wall-wart or panel-mount PSU, a DIN-rail unit is more compact, easier to wire,
> and industrial-grade reliable.

---

### DFRobot — Water Level Kit

| # | Part | Source | Qty | ~Price |
|---|------|--------|-----|--------|
| 8 | Gravity: Water Pressure/Level Sensor Kit (KIT0139) | [DFRobot KIT0139](https://www.dfrobot.com/product-1863.html) | 1 | ~$44 |

This kit includes both the submersible pressure sensor and the Gravity 4-20mA
converter board. The converter board includes overvoltage protection for the ESP32
ADC input — this is why we use the kit rather than wiring a DIY shunt resistor.

> **[?] 4–20 mA current loop:** The water level sensor outputs a current between
> 4 mA (empty) and 20 mA (full) on its signal wire. This is a standard industrial
> format. The DFRobot converter board turns that current into a 0–3.3V voltage that
> the ESP32 ADC can read.

---

### Amazon — Relay, Enclosure, Panel, Wiring

| # | Part | Notes | Qty | ~Price |
|---|------|-------|-----|--------|
| 9 | ESP32-S3 DevKitC-1 (38-pin, USB-C) | Search: "ESP32-S3 DevKitC-1 38 pin" | 1 | ~$10–12 |
| 10 | 8-channel relay module, opto-isolated | **Must be 3.3V compatible** — see Arrival Check below | 1 | ~$10 |
| 11 | IP65 waterproof enclosure, min 250×200×100mm | Confirm gasket is included | 1 | ~$20–30 |
| 12 | **GFCI outlet (20A)** or **DIN-rail RCD (30mA trip)** | Required — not optional | 1 | ~$15–40 |
| 13 | DIN rail 35mm, 30cm length | Hacksaw to fit enclosure width | 1 | ~$5 |
| 14 | DIN-rail blade fuse block, 6–8 position | One fuse per relay load channel | 1 | ~$12–15 |
| 15 | Panel switches: DPDT toggle (master) + SPST toggle ×4 | 5 switches total | 5 | ~$12 |
| 16 | Cable glands assorted: PG9 ×6, PG11 ×8, PG16 ×1 | Nylon, IP68 rated | assorted | ~$8 |
| 17 | Wago 221 lever connectors, 3-conductor | For mains splices inside enclosure | 1 pack | ~$10 |
| 18 | 22 AWG stranded wire, 5+ colours | Low-voltage signal wiring | 1 spool each | ~$15 |
| 19 | 14 AWG stranded wire, black + white + green | Mains wiring | 1 spool each | ~$10 |
| 20 | Ferrule crimp kit, 22 AWG + 14 AWG | End sleeves for screw terminals | 1 | ~$10 |
| 21 | DIN-rail terminal block strip, 10-position ×2 | For shared neutral and ground buses | 2 | ~$8 |
| 22 | Panel-mount fuse holder + 5A fuse | Mains input fuse (in-line, between GFCI and PSUs) | 1 | ~$5 |
| 23 | M3 standoffs, 10mm, 10× | Mounting ESP32 + relay board to enclosure floor | 1 bag | ~$3 |
| 24 | Zip ties / velcro cable ties | Cable management | — | ~$3 |
| 25 | Heat shrink tubing assorted | Insulating solder joints | 1 pack | ~$3 |
| 26 | Label maker + tape | Label every wire and component | — | (if you don't own one) |

---

### Electronics Supplier — Passive Components

Order from a resistor/capacitor kit, or from Mouser/Digi-Key individually:

| # | Part | Value | Qty needed | Notes |
|---|------|-------|-----------|-------|
| 27 | Resistor | 10 kΩ, ¼W | 10 | Relay IN pin pull-ups (8 used; 2 spare) |
| 28 | Resistor | 2.2 kΩ, ¼W | 2 | 1-Wire bus pull-up (1 used; 1 spare) |
| 29 | Ceramic capacitor | 100 nF (0.1 µF) | 2 | 1-Wire decoupling (1 used; 1 spare) |

> **[?] Pull-up resistor:** A resistor connected between a signal line and the
> positive supply voltage (VCC). It "pulls" the signal line to a HIGH state when
> nothing else is driving it. Without pull-ups, an undriven signal line can float
> to any random voltage — which in this build would randomly fire relays. The 10 kΩ
> resistors on the relay IN pins ensure every relay stays OFF until the ESP32
> actively tells it to turn on.

> **[?] Decoupling capacitor:** A small capacitor placed close to a signal line to
> absorb brief voltage spikes. The 100 nF cap on the 1-Wire bus suppresses "ringing"
> — oscillations that travel back down the cable and can corrupt data from the
> DS18B20 temperature probes.

---

## Optional Add-on: Reservoir Top-Off Pump

The base build pre-wires relay Ch7 and the 12V rail to support a top-off pump.
You do not need to add it now — it can be connected later with no enclosure
rewiring.

| # | Part | Notes | ~Price |
|---|------|-------|--------|
| A | 12V DC submersible pump, 3–5 L/min | Food/aquarium safe | ~$10 |
| B | 3/8" or 1/2" food-safe silicone tubing | To route water from supply to reservoir | ~$5 |
| C | Aquarium check valve, matching ID | Prevents backflow into pump | ~$5 |

---

## What to Verify on Arrival

Before starting Chapter 02, inspect every part as it arrives.

### Relay Module — Critical Checks

> ⚠️ **SAFETY:** A relay module that cannot reliably switch at 3.3V will drop out
> during a fruiting cycle — typically after hours of run time when the PCB is warm.
> This check happens now, not when a relay fails mid-grow.

**Check A — Locate the VCC/JD-VCC jumper.**
Find the 3-pin header on the relay board with a shorting jumper bridging two of the
pins. It is usually labelled VCC–JD-VCC or similar. Confirm the jumper is present
(you will remove it in Chapter 04).

**Check B — Verify 3.3V compatibility.**
One of these must be true:
- The product listing explicitly states "3.3V compatible" or "3.3V trigger"
- The IN pin current-limiting resistors measure ≤ 470 Ω (use your multimeter:
  one probe on an IN pin, other probe on the adjacent GND pin — you are reading
  the resistor in circuit, so expect the reading to be slightly low due to
  parallel paths)

If the resistors measure ~1 kΩ and the listing does not confirm 3.3V, you have two
options: replace the relay module with a confirmed 3.3V-compatible one, or swap the
1 kΩ IN pin resistors for 470 Ω (requires soldering on SMD components). Contact the
supplier before buying if in doubt.

> **[?] Why 3.3V matters:** The ESP32-S3 outputs logic HIGH at 3.3V. Standard relay
> modules expect 5V logic. The optocoupler LED inside the module needs enough current
> to reliably switch. At 5V with a 1 kΩ resistor: ~3.8 mA. At 3.3V with the same
> resistor: ~2.1 mA — below the reliable activation threshold. The relay may work
> at room temperature but drop out when warm. Using 470 Ω resistors (or a module
> rated for 3.3V) raises this to ~4.5 mA, which is within the reliable range.

**✓ Check:** You can locate the VCC/JD-VCC jumper. The board is confirmed 3.3V
compatible by listing or by resistor measurement.

---

### Enclosure — Critical Checks

**Check A — IP65 rating.**
Look for "IP65" printed or moulded on the enclosure or its data sheet. If it's not
on the enclosure, check the packaging. IP65 is the minimum — the gasket must be
intact and undamaged.

**Check B — Internal dimensions.**
Measure the interior. You need at least 250 × 200 mm floor area and 90 mm depth.
The DIN rail, two PSUs, relay board, and ESP32 all need to fit.

**Check C — Lid switch area.**
You will drill 5 switch holes in the lid. Confirm the lid has a flat area large
enough for five 12–16 mm holes spaced ~25 mm apart.

**✓ Check:** Enclosure is IP65 rated with intact gasket. Interior dimensions
confirmed. Lid has sufficient flat area for panel switches.

---

### GFCI / RCD — Critical Check

> ⚠️ **SAFETY:** The GFCI or RCD is the device that prevents electrocution in a
> wet environment. A 5A fuse will not protect you — it trips at ~5,000 mA; lethal
> current is ~100 mA. The GFCI/RCD trips at 5–30 mA in under 40 ms.

**Verify the trip rating is 30 mA (or less).** This is printed on the device.
A 100 mA or higher trip rating is not adequate for this application.

If using a DIN-rail RCD, confirm it is a Type A or Type F device (detects both AC
and pulsed DC residual current) rather than Type AC (AC only). Type A is standard
and widely available.

**✓ Check:** GFCI or RCD confirmed 30 mA trip rating. Device is undamaged. Test
button present and moves freely.

---

## Chapter 01 Checkpoint

Before moving to Chapter 02, confirm:

- [ ] All parts ordered or in hand
- [ ] Relay module confirmed 3.3V compatible (or action plan noted to fix)
- [ ] Enclosure confirmed IP65 with intact gasket and sufficient dimensions
- [ ] GFCI/RCD confirmed 30 mA trip rating
- [ ] All tools available (multimeter, soldering iron, wire strippers, ferrule
  crimper, drill + step bit, label maker)
- [ ] 10 kΩ resistors ×10, 2.2 kΩ resistors ×2, 100 nF capacitors ×2 confirmed
  in parts inventory
