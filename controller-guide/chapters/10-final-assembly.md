# Chapter 10 — Final Assembly Checklist

**What you'll do:** Complete three structured checklists (low-voltage, mains,
mechanical), photograph every zone, inspect the gasket, and close the enclosure.

**Prerequisites:** Chapters 02–09 complete. Label maker, camera or phone, torque
screwdriver or pliers for cable glands.

> ⚠️ **SAFETY:** The enclosure must be unplugged for this entire chapter. You are
> doing a final visual inspection of the mains zone — treat it as live even though
> it is currently unplugged.

---

## Before You Close the Lid

Once the lid is closed and sealed, access requires opening the IP65 gasket seal.
This chapter is your last chance to verify everything before committing.

Take your time. A missed connection found now costs 5 minutes. A missed connection
found after power-on may cost a component or a fuse.

---

## Checklist A — Low-Voltage Side (30 items)

**Relay board:**
- [ ] VCC/JD-VCC jumper confirmed removed
- [ ] Relay VCC connected to ESP32 3V3 (not 5V)
- [ ] Relay JD-VCC connected to 5V PSU positive output
- [ ] Relay GND connected to common GND
- [ ] Relay module confirmed 3.3V compatible (IN pin resistors ≤ 470 Ω)
- [ ] 10 kΩ pull-up resistors fitted on all 8 IN pins (IN1–IN8 to VCC rail)

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
- [ ] DPDT master: COM → common wire node; AUTO throw → 3V3; MANUAL throw → GND
- [ ] HUMIDITY switch: common wire → IN1 and IN2
- [ ] FAE switch: common wire → IN3 and IN4
- [ ] UVC switch: common wire → IN5
- [ ] LIGHTS switch: common wire → IN6
- [ ] Sufficient wire slack for lid to open without pulling connections

**Sensor wiring:**
- [ ] TCA9548A: VIN → 3V3; GND → GND; SDA → GPIO 21; SCL → GPIO 9; A0/A1/A2 → GND
- [ ] SCD30: VIN → 3V3; GND → GND; SDA → GPIO 21; SCL → GPIO 9; SEL → GND
- [ ] AS7341: VIN → 3V3 (confirmed not 5V); SDA → GPIO 21; SCL → GPIO 9
- [ ] SHT45 #1 → TCA9548A channel 0; labelled TOP
- [ ] SHT45 #2 → TCA9548A channel 1; labelled MID
- [ ] SHT45 #3 → TCA9548A channel 2; labelled BOT
- [ ] DS18B20 pull-up: 2.2 kΩ between GPIO 4 and 3V3
- [ ] DS18B20 decoupling cap: 100 nF at pull-up junction to GND
- [ ] All 5 DS18B20 probes on GPIO 4; each labelled S1–S5
- [ ] KIT0139 converter board: OUT → GPIO 7; VCC → 3V3; GND → GND
- [ ] KIT0139 sensor: 12V PSU + → sensor supply; 12V PSU − → sensor GND
- [ ] All I2C devices confirmed powered from 3V3, not 5V PSU
- [ ] Common GND: 5V PSU −, 12V PSU −, and ESP32 GND all tied to single bus

---

## Checklist B — Mains Side (15 items)

> ⚠️ **SAFETY:** Visual inspection of mains zone. Do not power on until all 15 items
> are checked.

- [ ] GFCI/RCD: mains input → LINE terminals (not LOAD terminals)
- [ ] GFCI/RCD: confirmed 30 mA trip rating
- [ ] Hot (Line) conductor connected to GFCI LINE-L
- [ ] Neutral conductor connected to GFCI LINE-N
- [ ] Earth conductor connected to GFCI PE terminal and/or enclosure chassis
- [ ] GFCI LOAD-L runs through input fuse (5A) before reaching PSUs or loads
- [ ] Ferrule crimped on every wire end in every mains terminal — no bare strands visible
- [ ] All Wago 221 lever connectors fully closed (lever down, wire locked)
- [ ] Both PSUs have L, N, PE connected
- [ ] Both PSU PE terminals connected to Earth bus
- [ ] Each relay COM terminal has Hot via correctly rated fuse (2A or 3A per channel)
- [ ] Each relay NO terminal has correct load cable Hot wire connected
- [ ] All load Neutral wires on Neutral bus
- [ ] All earthed load Earth wires on Earth bus
- [ ] Relay board: load terminals (COM/NO) are in mains zone; signal header in LV zone

---

## Checklist C — Enclosure Mechanical (10 items)

- [ ] All cable glands tightened: gland nut grips the cable jacket; IP65 seal requires
  the rubber clamping the cable, not floating around it
- [ ] Mains input cable gland (PG16) tightened
- [ ] All load cable glands (PG11) tightened
- [ ] All sensor cable glands (PG9) tightened
- [ ] No empty cable gland holes (if a gland has no cable, insert a blanking plug
  to maintain IP65 rating)
- [ ] Lid gasket: inspect all the way around — no sections missing, no gaps, gasket
  seated in groove without bunching
- [ ] All panel switch mounting nuts tight
- [ ] ESP32-S3 mounting screws secure (no movement)
- [ ] Relay board mounting screws secure (no movement)
- [ ] DIN rail mounting screws secure (no wobble)

---

## Photography Checkpoint

Before sealing the lid, take photos of:

1. **Mains zone — wide shot:** Shows GFCI, fuse holder, fuse block, PSUs, and
   all mains wiring in context
2. **Mains zone — close-up of GFCI terminals:** Shows LINE vs LOAD connections,
   confirming correct input side
3. **Mains zone — close-up of relay load terminals:** Shows COM/NO connections
   and fuse block connections
4. **LV zone — wide shot:** Shows ESP32, relay signal header, sensor boards
5. **LV zone — close-up of relay signal header:** Shows IN1–IN8 and VCC/JD-VCC
   connections
6. **Lid interior — failsafe switches:** Shows DPDT and SPST wiring
7. **Panel exterior — lid with switch labels visible**

These photos are your maintenance documentation. Store them with the project files.

---

## Closing the Enclosure

1. Double-check all sensor cable routes — no cable is pinched between the lid and
   the enclosure body.
2. Tuck any excess cable into neat bundles using cable ties or velcro.
3. Lower the lid and seat the gasket. The gasket should compress evenly around the
   perimeter — no section should be raised or gapped.
4. Close all lid latches or tighten lid bolts in a cross-pattern (like tightening
   lug nuts) to ensure even gasket compression.

**✓ Check:** Lid is closed and latched/bolted. Gasket is compressed evenly around
the full perimeter.

---

## Chapter 10 Checkpoint

- [ ] Checklist A — all 30 items complete
- [ ] Checklist B — all 15 items complete
- [ ] Checklist C — all 10 items complete
- [ ] 7 photos taken and stored
- [ ] Lid closed and sealed with gasket properly seated
