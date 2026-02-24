# Chapter 02 — Enclosure Preparation

**What you'll do:** Mark the two-zone layout, drill cable gland holes and switch
holes, install cable glands, cut and mount the DIN rail, and pre-drill standoff
mounting holes.

**Prerequisites:** Empty enclosure, drill + step bit, marker/scribe, ruler, cable
glands (PG9, PG11, PG16), DIN rail + screws, label maker.

> ⚠️ **SAFETY:** The enclosure is empty and unplugged. No mains voltage is involved
> in this chapter. Standard workshop precautions apply: wear eye protection when
> drilling and deburring metal.

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
│  • Meanwell 5V 3A PSU           │  • 8-channel relay module       │
│  • Meanwell 12V 1A PSU          │  • TCA9548A, SCD30, AS7341      │
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
> enclosure and know that nothing there is above 12V DC. It also satisfies good
> electrical practice (IEC 60364 and similar codes require separation of circuits
> of different voltage classes).

---

## Step 1 — Mark the Zone Divider

1. Open the enclosure lid and set it aside.
2. With a marker or scribe, draw a vertical line roughly 40% of the way across the
   floor and walls of the enclosure. This is your **zone divider line**.
3. Label the left zone "MAINS" and the right zone "LV" using a label maker or
   marker. Make the labels large enough to be visible when the enclosure is open.

**✓ Check:** Zone divider line is visible. Both zones are labelled.

---

## Step 2 — Plan Cable Gland Positions

Sketch the gland layout before drilling. Once you drill, you can't un-drill.

**Placement rules:**
- All mains cable glands on the **bottom edge** of the enclosure, in the mains zone
- Sensor cable glands on the **bottom edge or side**, in the LV zone
- Glands should enter **from below** where possible — water runs down, not up,
  so a downward-entry gland can't fill with drips

**Gland sizing:**

| Gland size | Fits cable OD | Use for |
|------------|--------------|---------|
| PG9 | 4–8 mm | Sensor cables (DS18B20, SHT45, water level) |
| PG11 | 5–10 mm | Load cables (fan, fogger, UVC, lights) |
| PG16 | 10–14 mm | Mains input cable (14 AWG 3-conductor) |

**Planned positions (adapt to your specific enclosure):**
- Bottom, mains zone, left: **1× PG16** — mains input cable
- Bottom, mains zone, right of PG16: **6× PG11** — one per load (fogger, tub fan,
  exhaust, intake, UVC, grow lights)
- Bottom or right side, LV zone: **6× PG9** — sensor cables (DS18B20 ×5 daisy
  chain counts as 1 cable; SHT45 cables; SCD30; AS7341; water level)

> **[?] Cable gland:** A threaded fitting that seals around a cable as it passes
> through a hole in the enclosure wall, maintaining the IP65 weather seal. The gland
> body threads into the hole; the clamping nut is tightened to grip the cable jacket
> and compress the rubber seal. The cable must be seated before final tightening.

---

## Step 3 — Drill Cable Gland Holes

> ⚠️ **SAFETY:** Wear eye protection. Step bits cut cleanly but create sharp swarf.
> Clamp the enclosure or hold it securely — do not hand-hold it against a spinning
> bit.

**Step bit technique:**

1. Mark each hole centre with a punch or sharp marker.
2. Start at low speed. Let the bit walk through each step — do not force it.
3. **Stop at each step and check:** step bits cut in increments. Check the gland
   thread size against the hole as you go. It is better to check at 18 mm and
   find you need 20 mm than to go to 25 mm.
4. Deburr every hole with a round file or deburring tool before installing a gland.
   Sharp edges will cut through cable jackets over time.

**Hole sizes:**

| Gland | Thread size | Hole diameter |
|-------|------------|---------------|
| PG9 | PG9 (M16×1.5 body) | 20–21 mm |
| PG11 | PG11 (M20×1.5 body) | 22–23 mm |
| PG16 | PG16 (M25×1.5 body) | 27–28 mm |

> **Note:** Hole sizes vary slightly by brand. Check the gland datasheet or
> thread the gland into a test hole before drilling the final position.

**✓ Check:** All holes drilled and deburred. Glands thread into holes by hand
without binding.

---

## Step 4 — Drill Switch Holes in the Lid

You need 5 holes for the failsafe panel switches: one DPDT master toggle and four
SPST group toggles.

1. Lay out the switch positions on the lid exterior. Space them at least 25 mm
   centre-to-centre. Keep all 5 within the middle half of the lid (away from
   corners where the lid's clamping hardware sits).
2. Mark and punch each hole centre.
3. Drill to the size specified in your toggle switch datasheet — typically 12 mm
   for standard mini-toggle switches.
4. Deburr each hole.
5. Test-fit each switch. The bushing should pass through and the nut should thread
   on from inside.

**Label positions (left to right is the convention used in this guide):**

```
[ MASTER AUTO/MAN ]  [ HUMIDITY ]  [ FAE ]  [ UVC ]  [ LIGHTS ]
```

Use a label maker to apply labels below each switch hole before installing the
switches. It is much easier to apply labels to the flat lid surface than around a
mounted switch.

**✓ Check:** All 5 holes drilled, deburred, and labelled. Switches fit through
holes and nut threads on.

---

## Step 5 — Install Cable Glands (Finger-Tight)

Thread each cable gland into its hole from outside. Tighten by hand until snug,
then a quarter-turn more with pliers — just enough to compress the thread seal.

**Do not fully tighten yet.** Cable glands grip the cable when the clamping nut is
tightened. They must be loose enough for cables to pass through freely until cables
are seated in Chapter 09 (mains) and Chapter 06 (sensors).

Mark each gland with a small sticker or tape flag indicating which cable goes
through it. Example: "MAINS IN", "FOGGER", "EXHAUST", "S1–DS18" etc.

**✓ Check:** All cable glands installed. Each cable gland turns freely (cable entry
side can be loosened and re-tightened).

---

## Step 6 — Cut and Mount DIN Rail

> **[?] DIN rail:** A standardised 35mm-wide metal channel used to mount
> industrial components inside control enclosures. PSUs, relay modules, fuse
> blocks, and terminal blocks all clip onto it. Saves drilling individual holes
> for every component.

1. Measure the interior width of the enclosure.
2. Cut the DIN rail 10–15 mm shorter than the interior width so it fits with
   clearance. Use a hacksaw; file the cut ends smooth so they don't cut your
   hands or cable insulation.
3. Mark two mounting hole positions — one near each end of the rail, at least
   20 mm from the cut ends.
4. Drill the rail with a metal drill bit at the marked positions (typically 4 mm
   for M4 screws; check your enclosure floor threads).
5. Mount the rail horizontally in the lower half of the enclosure floor, with enough
   clearance above it for components to clip on and below it for wiring to run.

If your enclosure floor has no mounting threads, use self-tapping screws or drill
and tap M4 holes.

**✓ Check:** DIN rail is secure (no wobble when pulled side-to-side). Components
clipped to the rail sit level.

---

## Step 7 — Pre-Drill Standoff Mounting Holes

The ESP32-S3 board and the relay module will be mounted on M3 standoffs, off the
enclosure floor.

1. Position the relay module in the low-voltage zone, near the zone divider line.
   Mark the four corner mounting hole positions on the enclosure floor.
2. Position the ESP32-S3 to the right of the relay module (away from the relay
   load terminals). Mark its four corner mounting hole positions.
3. Drill M3 holes at all 8 marked positions. Tap for M3 threads, or use self-
   tapping M3 standoffs.
4. Thread standoffs into the holes. Use 10–15 mm standoffs to give clearance
   between the board and the enclosure floor.

**✓ Check:** Standoffs are secure. ESP32 board placed on standoffs does not rock.
Relay module placed on standoffs does not rock.

---

## Chapter 02 Checkpoint

- [ ] Zone divider line marked; zones labelled MAINS and LV
- [ ] All cable gland holes drilled and deburred
- [ ] 5 switch holes drilled, deburred, and labelled on lid
- [ ] Cable glands installed (finger-tight)
- [ ] DIN rail cut, filed, and mounted securely
- [ ] Standoff holes drilled for ESP32 and relay module
- [ ] Standoffs installed and stable
