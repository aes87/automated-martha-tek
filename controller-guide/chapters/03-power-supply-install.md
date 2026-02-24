# Chapter 03 — Power Supply Installation (Mechanical)

**What you'll do:** Mount the GFCI/RCD, both PSUs, fuse holders, and fuse block
on the DIN rail. Label every component. No wiring in this chapter.

**Prerequisites:** Completed Chapter 02. GFCI/RCD, Meanwell PSUs, fuse holder,
blade fuse block, label maker.

> ⚠️ **SAFETY:** Enclosure is unplugged. No mains voltage is involved in this
> chapter. This chapter is mechanical placement and labelling only. Wiring
> happens in Chapter 09.

---

## Why Label Before Wiring

Labelling components before wiring sounds backwards, but it is the right order:

- Labels are much easier to apply to a clean surface than to a wired one
- A labelled enclosure makes the wiring chapter faster — you can see at a glance
  what goes where
- If you return to the enclosure months later for maintenance, labels are the
  first thing you'll thank yourself for

Use a label maker if you have one. Dymo-style or Brother P-Touch labels are both
fine. Permanent marker on masking tape also works.

---

## DIN Rail Component Order

Mount components on the DIN rail from left to right in this order:

```
[GFCI/RCD] → [Fuse Holder 5A] → [Blade Fuse Block] → [5V PSU] → [12V PSU]
```

The GFCI/RCD **must** be the first device the mains feed reaches — before the
fuse, before the PSUs, before anything. This is not just convention; it is a safety
requirement. Everything else on the enclosure must be downstream of the GFCI.

> ⚠️ **SAFETY:** A GFCI/RCD trips at 5–30 mA ground fault current — fast enough
> to prevent electrocution. A fuse trips at thousands of mA — far too slow. Placing
> the GFCI after the fuse would mean a ground fault could occur in the wiring
> between the mains inlet and the fuse, bypassing the GFCI protection entirely.
> First device = GFCI. Always.

---

## Step 1 — Mount the GFCI/RCD

**If using a DIN-rail RCD (preferred for this build):**

1. Clip the RCD onto the leftmost position of the DIN rail.
2. The RCD has a mains IN side (line and neutral) and a mains OUT side. Orient it
   so the IN side faces the mains inlet side of the enclosure (the PG16 gland you
   drilled in Chapter 02).
3. Apply a label: `GFCI/RCD — 30mA — FIRST DEVICE`

**If using a GFCI outlet (alternative):**

A GFCI outlet can be panel-mounted on the enclosure face or wall-mounted inside.
Ensure its LINE terminals will be the first connection on the mains feed.

**✓ Check:** GFCI/RCD is in the leftmost position on DIN rail. Label applied.

---

## Step 2 — Mount the Input Fuse Holder

1. Clip or mount the panel-mount fuse holder immediately to the right of the
   GFCI/RCD on the DIN rail (or mount it on the enclosure floor near the DIN rail
   if it is not a DIN-rail type).
2. Insert a 5A fast-blow fuse.
3. Apply a label: `MAIN FUSE — 5A`

> **[?] Why a 5A fuse on the input when there are per-load fuses too?** The main
> fuse protects the mains input cable and wiring inside the enclosure from a fault
> between the GFCI and the fuse block. The per-load fuses (Step 3) protect
> individual channels. Together they give defence-in-depth: a fault in the fogger
> cable blows the 2A fogger fuse, not the 5A main fuse.

**✓ Check:** Fuse holder mounted and labelled. 5A fuse installed.

---

## Step 3 — Mount the Blade Fuse Block

1. Clip the blade fuse block onto the DIN rail, to the right of the fuse holder.
2. Install blade fuses in each position:

| Position | Load | Fuse rating |
|----------|------|------------|
| 1 | Fogger | 2A |
| 2 | Tub fan | 2A |
| 3 | Exhaust fan | 2A |
| 4 | Intake fan | 2A |
| 5 | UVC lights | 2A |
| 6 | Grow lights | 3A |

   Fuse positions 7–8 can remain empty (Ch7 pump and Ch8 spare are not in base build).

3. Apply a label to each position using the fuse block's built-in card slot (if
   present) or a label maker strip: "FOGGER 2A", "TUB FAN 2A", etc.

> **[?] Fuse rating rule:** Fuse at 125% of normal load current, rounded up to the
> next standard value. Most fans in this build draw 0.5–1.5A; foggers typically
> draw under 1A; LED lights depend on strip wattage. A 2A fuse provides a 2–4×
> margin while still protecting the load wiring.

**✓ Check:** Fuse block mounted and labelled. Correct fuse values installed in
each position.

---

## Step 4 — Mount the 5V PSU (Meanwell HDR-30-5)

1. Clip the 5V PSU onto the DIN rail, to the right of the fuse block.
2. Apply a label to the top face: `5V 3A — LOGIC`

The 5V PSU powers:
- The ESP32-S3 (via its VIN pin or USB-C)
- The relay module coils (JD-VCC)
- Any 5V sensor boards (though all sensors in this build are 3.3V, the 5V rail
  is available if needed)

Note the PSU's terminal blocks: usually L (line/hot), N (neutral), PE (earth) on
the mains input side, and + and − on the DC output side. Orient the PSU so these
terminals are accessible.

**✓ Check:** 5V PSU mounted and labelled. L/N/PE terminals and + / − terminals
are accessible.

---

## Step 5 — Mount the 12V PSU (Meanwell HDR-15-12)

1. Clip the 12V PSU onto the DIN rail, to the right of the 5V PSU.
2. Apply a label: `12V 1A — WATER LEVEL`

The 12V PSU powers:
- The DFRobot KIT0139 water level sensor (sensor supply)
- The optional top-off pump (if added later via relay Ch7)

**✓ Check:** 12V PSU mounted and labelled. Terminals are accessible.

---

## Step 6 — Check the Full Layout

With all components mounted, step back and verify the layout makes sense before
any wiring starts.

```
[GFCI/RCD] [5A FUSE] [FUSE BLOCK] [5V PSU] [12V PSU]
     ↑                                          ↑
Leftmost —                           — Rightmost on DIN rail
Mains in first                       Mains zone right edge
```

The DIN rail should still have room to the right for a row of terminal blocks
if needed. If not, a second DIN rail or a terminal block DIN strip can be added
below.

**✓ Check:** Components mounted in correct order (GFCI first). Sufficient gap
between components for wiring access. Zone divider line is still respected —
all mounted components are in the mains zone.

---

## Step 7 — Label the Zones on the DIN Rail Itself

Apply a strip of red tape or a large label along the mains zone section of the
DIN rail: "HIGH VOLTAGE — UNPLUG BEFORE TOUCHING"

Apply a strip of green tape or a label along the LV zone section: "LOW VOLTAGE ≤5V"

These labels serve as a reminder every time the lid is opened.

**✓ Check:** Zones labelled on DIN rail. Labels are clear and not covering any
component terminals.

---

## Chapter 03 Checkpoint

- [ ] GFCI/RCD mounted as first (leftmost) component on DIN rail
- [ ] Input fuse holder mounted with 5A fuse installed
- [ ] Blade fuse block mounted with correct per-load fuse ratings
- [ ] 5V 3A PSU mounted and labelled
- [ ] 12V 1A PSU mounted and labelled
- [ ] All components labelled before wiring begins
- [ ] DIN rail zones labelled HIGH VOLTAGE / LOW VOLTAGE
- [ ] No wiring done yet — mechanical and labelling only
