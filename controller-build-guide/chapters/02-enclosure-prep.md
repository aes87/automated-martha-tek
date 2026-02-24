# Chapter 02 — Enclosure Preparation

**What you'll do:** Mark the two-zone layout, drill cable gland holes and switch
holes, install cable glands, cut and mount the DIN rail, and pre-drill standoff
mounting holes.

**Prerequisites:** Empty IP65 enclosure. Drill + step bit, marker, ruler, cable
glands, DIN rail + screws, label maker.

> ⚠️ **SAFETY:** The enclosure is empty and unplugged. No mains voltage in this
> chapter. Wear eye protection when drilling and deburring metal.

---

## Understanding the Two-Zone Layout

The enclosure interior is divided into two logical zones. No physical barrier is
needed — the zones are maintained by discipline during assembly.

```
┌─────────────────────────────────────────────────────────────────┐
│                                                                   │
│  MAINS ZONE (left ~40%)        │  LOW-VOLTAGE ZONE (right ~60%) │
│                                 │                                 │
│  Contains:                      │  Contains:                      │
│  • GFCI/RCD                     │  • ESP32-S3 DevKitC-1           │
│  • 5V 3A PSU                    │  • 8-channel relay module       │
│  • 12V 1A PSU                   │  • TCA9548A, SCD30, AS7341      │
│  • Input fuse holder (5A)       │    sensor breakout boards       │
│  • Blade fuse block (per-load)  │  • Terminal blocks (LV signals) │
│  • Mains terminal blocks        │                                 │
│  • All mains wiring (120/230V)  │  Everything here is ≤ 5V DC     │
│                                 │                                 │
│  ← Loads exit via cable glands  │  ← Sensors enter via PG9 glands │
│    on bottom edge               │    on side or bottom edge        │
│─────────────────────────────────│                                 │
│  ⚠️ HIGH VOLTAGE — NEVER OPEN  │  The relay board's coil/signal  │
│  WHILE PLUGGED IN               │  header is in this zone; its    │
│                                 │  load contacts bridge left      │
└─────────────────────────────────────────────────────────────────┘
```

The relay board physically spans both zones: its load terminals (COM/NO/NC contacts
that switch mains) point toward the mains zone; its signal header (IN1–IN8, VCC,
GND) faces the low-voltage zone. Mount it at or near the dividing line.

> **[?] Why two zones?** Mixing low-voltage and mains wiring in the same space
> creates risk of accidental contact and makes fault-finding confusing. The physical
> separation means that during maintenance you can look at the right half of the
> enclosure and know that nothing there is above 12V DC.

---

## Step 1 — Mark the Zone Divider

1. Open the enclosure lid and set it aside.
2. With a marker or scribe, draw a vertical line roughly 40% of the way across the
   floor and walls of the enclosure. This is your **zone divider line**.
3. Label the left zone "MAINS" and the right zone "LV" with a label maker or marker.

**✓ Check:** Zone divider line is visible. Both zones are labelled.

---

## Step 2 — Plan Cable Gland Positions

Sketch the gland layout before drilling. Once you drill, you can't un-drill.

**Placement rules:**
- All mains cable glands on the **bottom edge** of the enclosure, in the mains zone
- Sensor cable glands on the **bottom edge or side**, in the LV zone
- Glands should enter **from below** where possible — water runs down, not up

**Gland sizing:**

| Gland size | Fits cable OD | Use for |
|------------|--------------|---------|
| PG9 | 4–8 mm | Sensor cables (DS18B20, SHT45, water level) |
| PG11 | 5–10 mm | Load cables (fan, fogger, UVC, lights) |
| PG16 | 10–14 mm | Mains input cable (14 AWG 3-conductor) |

**Planned positions (adapt to your specific enclosure):**
- Bottom, mains zone, left: **1× PG16** — mains input cable
- Bottom, mains zone: **6× PG11** — one per load (fogger, tub fan, exhaust, intake,
  UVC, grow lights)
- Bottom or right side, LV zone: **6× PG9** — sensor cables

> **[?] Cable gland:** A threaded fitting that seals around a cable as it passes
> through a hole in the enclosure wall, maintaining the IP65 weather seal. The gland
> body threads into the hole; the clamping nut is tightened to grip the cable jacket
> and compress the rubber seal.

---

## Step 3 — Drill Cable Gland Holes

> ⚠️ **SAFETY:** Wear eye protection. Step bits create sharp swarf. Clamp or
> hold the enclosure securely.

**Step bit technique:**

1. Mark each hole centre with a punch or sharp marker.
2. Start at low speed. Let the bit walk through each step — do not force it.
3. Stop at each step and check the hole size against your gland before going deeper.
4. Deburr every hole with a round file before installing a gland. Sharp edges will
   cut through cable jackets over time.

**Hole sizes:**

| Gland | Thread size | Hole diameter |
|-------|------------|---------------|
| PG9 | M16×1.5 | 20–21 mm |
| PG11 | M20×1.5 | 22–23 mm |
| PG16 | M25×1.5 | 27–28 mm |

> **Note:** Verify against your specific gland's datasheet — sizes vary slightly
> by brand. Thread a gland into a test hole before drilling the final position.

**✓ Check:** All holes drilled and deburred. Glands thread into holes by hand
without binding.

---

## Step 4 — Drill Switch Holes in the Lid

You need 5 holes for the failsafe panel switches (1× DPDT master, 4× SPST group).

1. Lay out 5 switch positions on the lid exterior, spaced at least 25 mm centre-
   to-centre. Keep them in the middle half of the lid, away from corner hardware.
2. Mark and punch each hole centre.
3. Drill to the diameter specified in your switch's datasheet — typically 12 mm for
   standard mini-toggle switches.
4. Deburr each hole. Test-fit each switch.

**Label positions from left to right:**

```
[ MASTER AUTO/MAN ]  [ HUMIDITY ]  [ FAE ]  [ UVC ]  [ LIGHTS ]
```

Apply labels below each hole **before** mounting the switches — it's much easier to
label a flat surface than to label around a mounted toggle.

**✓ Check:** All 5 holes drilled, deburred, labelled. Switches fit through holes
and mounting nuts thread on.

---

## Step 5 — Install Cable Glands (Finger-Tight)

Thread each cable gland into its hole from outside. Tighten finger-tight plus a
quarter-turn — just enough to compress the thread seal.

**Do not fully tighten yet.** The clamping nut must remain loose until cables are
seated. Label each gland with a tape flag indicating which cable goes through it:
"MAINS IN", "FOGGER", "EXHAUST", "SHT45-TOP", etc.

**✓ Check:** All cable glands installed and finger-tight. Each is labelled.

---

## Step 6 — Cut and Mount DIN Rail

> **[?] DIN rail:** A standardised 35mm-wide metal channel used to mount industrial
> components. PSUs, relay modules, fuse blocks, and terminal blocks all clip onto it.

1. Measure the interior width of the enclosure.
2. Cut the DIN rail 10–15 mm shorter than the interior width. Use a hacksaw; file
   the cut ends smooth.
3. Mark two mounting holes — one near each end of the rail, at least 20 mm from
   the cut ends.
4. Drill the rail and mount it horizontally in the lower half of the enclosure floor,
   with clearance above for components and below for wiring.

**✓ Check:** DIN rail is secure with no wobble. Components clip and sit level.

---

## Step 7 — Pre-Drill Standoff Mounting Holes

The ESP32-S3 and relay module mount on M3 standoffs off the enclosure floor.

1. Position the relay module in the LV zone near the zone divider. Mark its four
   corner mounting holes on the enclosure floor.
2. Position the ESP32-S3 to the right of the relay module. Mark its four corner
   mounting holes.
3. Drill M3 holes at all 8 positions. Tap for M3 threads or use self-tapping
   M3 standoffs.
4. Install standoffs (10–15 mm). Verify boards sit stable with no rocking.

**✓ Check:** Standoffs are secure. Both boards sit flat when placed on standoffs.

---

## Chapter 02 Checkpoint

- [ ] Zone divider line marked; zones labelled MAINS and LV
- [ ] All cable gland holes drilled and deburred
- [ ] 5 switch holes drilled, deburred, and labelled on lid
- [ ] Cable glands installed finger-tight and labelled
- [ ] DIN rail cut, filed, and mounted securely
- [ ] Standoff holes drilled for ESP32 and relay module
- [ ] Standoffs installed and stable

---

[← Ch 01 — Parts & Tools](01-parts-and-tools.md)  ·  [Ch 03 — Power Supply Installation →](03-power-supply-install.md)
