# Chapter 10 — Final Assembly Checklist

**What you'll do:** Complete three structured checklists (low-voltage, mains,
mechanical), photograph every zone, inspect the gasket, and close the enclosure.

**Prerequisites:** Chapters 02–09 complete. Label maker, camera, pliers for glands.

> ⚠️ **SAFETY:** Enclosure unplugged. This is your last inspection of the mains zone
> before sealing — treat it as live even though it is currently unplugged.

---

## Before You Close the Lid

Once sealed, access requires breaking the IP65 gasket seal. This chapter is your
last chance to verify everything. Take your time.

---

## Checklist A — Low-Voltage Side (30 items)

**Relay board:**
- [ ] VCC/JD-VCC jumper confirmed removed
- [ ] Relay VCC → ESP32 3V3 (not 5V)
- [ ] Relay JD-VCC → 5V PSU positive output
- [ ] Relay GND → common GND
- [ ] IN pin resistors confirmed ≤ 470 Ω (3.3V compatible)
- [ ] 10 kΩ pull-up resistors on all 8 IN pins

**Signal wiring:**
- [ ] GPIO 38 → Relay IN1 (Fogger)
- [ ] GPIO 39 → Relay IN2 (Tub fan)
- [ ] GPIO 18 → Relay IN3 (Exhaust fan)
- [ ] GPIO 19 → Relay IN4 (Intake fan)
- [ ] GPIO 40 → Relay IN5 (UVC lights)
- [ ] GPIO 41 → Relay IN6 (Grow lights)
- [ ] GPIO 42 → Relay IN7 (Pump — pre-wired)
- [ ] GPIO 47 → Relay IN8 (Spare)
- [ ] All signal wires labelled at both ends

**Failsafe panel:**
- [ ] DPDT: COM → common wire node; AUTO → 3V3; MANUAL → GND
- [ ] HUMIDITY switch → IN1 and IN2
- [ ] FAE switch → IN3 and IN4
- [ ] UVC switch → IN5
- [ ] LIGHTS switch → IN6
- [ ] Sufficient wire slack for lid to open without pulling connections

**Sensor wiring:**
- [ ] TCA9548A: VIN→3V3; SDA→GPIO 21; SCL→GPIO 9; A0/A1/A2→GND
- [ ] SCD30: VIN→3V3; SDA/SCL on main bus; SEL→GND
- [ ] AS7341: VIN→3V3 (confirmed not 5V); SDA/SCL on main bus
- [ ] SHT45 #1 → TCA9548A ch0; labelled TOP
- [ ] SHT45 #2 → TCA9548A ch1; labelled MID
- [ ] SHT45 #3 → TCA9548A ch2; labelled BOT
- [ ] DS18B20 pull-up: 2.2 kΩ between GPIO 4 and 3V3
- [ ] DS18B20 decoupling cap: 100 nF at pull-up junction to GND
- [ ] All 5 DS18B20 probes on GPIO 4; each labelled S1–S5
- [ ] KIT0139: OUT→GPIO 7; VCC→3V3; sensor supply from 12V PSU
- [ ] All I2C devices from 3V3, not 5V PSU
- [ ] Common GND: 5V PSU −, 12V PSU −, ESP32 GND all tied together

---

## Checklist B — Mains Side (15 items)

> ⚠️ **SAFETY:** Visual inspection only. Do not power on until all 15 items pass.

- [ ] GFCI/RCD: mains input → LINE terminals (not LOAD)
- [ ] GFCI/RCD: confirmed 30 mA trip rating
- [ ] Hot → GFCI LINE-L; Neutral → GFCI LINE-N; Earth → GFCI PE
- [ ] GFCI LOAD-L through input fuse (5A) before reaching PSUs or loads
- [ ] Ferrule on every mains wire end — no bare strands
- [ ] All Wago 221 levers fully closed
- [ ] Both PSUs have L, N, PE connected
- [ ] Both PSU PE → Earth bus
- [ ] Each relay COM has Hot via correctly rated fuse (2A or 3A per channel)
- [ ] Each relay NO has correct load cable Hot wire
- [ ] All load Neutrals on Neutral bus
- [ ] All earthed load Earths on Earth bus
- [ ] Relay load terminals (COM/NO) in mains zone; signal header in LV zone
- [ ] Earth continuity: mains Earth → chassis → both PSU PE terminals (multimeter)
- [ ] No bare wire visible at any mains terminal

---

## Checklist C — Enclosure Mechanical (10 items)

- [ ] PG16 mains input gland tightened (rubber seal gripping cable jacket)
- [ ] All PG11 load cable glands tightened
- [ ] All PG9 sensor cable glands tightened
- [ ] No empty gland holes (blanking plug in any unused hole to maintain IP65)
- [ ] Lid gasket seated fully around perimeter — no gaps, no bunching
- [ ] All 5 panel switch mounting nuts tight
- [ ] ESP32-S3 mounting screws secure
- [ ] Relay board mounting screws secure
- [ ] DIN rail mounting screws secure
- [ ] No cable pinched between lid and enclosure body

---

## Photography Checkpoint

Before sealing, photograph:

1. **Mains zone — wide shot:** GFCI, fuses, PSUs, and mains wiring in context
2. **Mains zone — GFCI terminal close-up:** LINE vs LOAD connections visible
3. **Mains zone — relay load terminals close-up:** COM/NO connections and fuse block
4. **LV zone — wide shot:** ESP32, relay signal header, sensor boards
5. **LV zone — relay signal header close-up:** IN1–IN8 and VCC/JD-VCC connections
6. **Lid interior — failsafe switches:** DPDT and SPST wiring
7. **Panel exterior — lid with switch labels**

Store photos with the project files.

---

## Closing the Enclosure

1. Verify all sensor cables are not pinched at the lid edge.
2. Tuck excess cables into neat bundles with cable ties.
3. Lower the lid and seat the gasket evenly around the full perimeter.
4. Close lid latches or tighten lid bolts in a cross-pattern for even compression.

**✓ Check:** Lid closed and latched. Gasket compressed evenly all the way around.

---

## Chapter 10 Checkpoint

- [ ] Checklist A — all 30 items complete
- [ ] Checklist B — all 15 items complete
- [ ] Checklist C — all 10 items complete
- [ ] 7 photos taken and stored
- [ ] Lid closed and sealed

---

[← Ch 09 — Mains Wiring](09-mains-wiring.md)  ·  [Ch 11 — Initial Power-On Test →](11-power-on-test.md)
