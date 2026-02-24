# Chapter 03 — Power Supply Installation (Mechanical)

**What you'll do:** Mount the GFCI/RCD, both PSUs, fuse holders, and fuse block
on the DIN rail. Label every component. No wiring in this chapter.

**Prerequisites:** Chapter 02 complete. GFCI/RCD, 5V 3A PSU, 12V 1A PSU, fuse
holder, blade fuse block, label maker.

> ⚠️ **SAFETY:** Enclosure is unplugged. This chapter is mechanical placement and
> labelling only. Wiring happens in Chapter 09.

---

## Why Label Before Wiring

Labels are far easier to apply to clean component surfaces than to wired ones.
A labelled enclosure makes the wiring chapter faster and makes future maintenance
self-explanatory.

Use a label maker (Dymo or Brother P-Touch). Permanent marker on tape also works.

---

## DIN Rail Component Order

Mount components left to right in this order:

```
[GFCI/RCD] → [Fuse Holder 5A] → [Blade Fuse Block] → [5V PSU] → [12V PSU]
```

The GFCI/RCD **must** be the first device the mains feed reaches — before the
fuse, before the PSUs, before anything else.

> ⚠️ **SAFETY:** The GFCI/RCD protects against electrocution in a wet environment.
> It must be the first device on the mains feed so that all wiring inside the
> enclosure is downstream of its protection. Placing it after the fuse would leave
> the input wiring unprotected.

---

## Step 1 — Mount the GFCI/RCD

1. Clip the RCD onto the leftmost position of the DIN rail. Orient it so the input
   (LINE) side faces the mains inlet cable gland.
2. Label: `GFCI/RCD — 30mA — FIRST DEVICE`

**✓ Check:** GFCI/RCD is in the leftmost position. Label applied.

---

## Step 2 — Mount the Input Fuse Holder

1. Mount the fuse holder immediately to the right of the GFCI/RCD.
2. Insert the 5A fuse.
3. Label: `MAIN FUSE — 5A`

> **[?] Two fusing levels:** The 5A input fuse protects the mains cable and internal
> wiring between the GFCI and the fuse block. The per-load fuses (next step) protect
> individual load channels. Together they provide defence-in-depth.

**✓ Check:** Fuse holder mounted and labelled. 5A fuse installed.

---

## Step 3 — Mount the Blade Fuse Block

1. Clip the blade fuse block onto the DIN rail to the right of the fuse holder.
2. Install blade fuses:

| Position | Load | Fuse rating |
|----------|------|------------|
| 1 | Fogger | 2A |
| 2 | Tub fan | 2A |
| 3 | Exhaust fan | 2A |
| 4 | Intake fan | 2A |
| 5 | UVC lights | 2A |
| 6 | Grow lights | 3A |

3. Label each position: "FOGGER 2A", "TUB FAN 2A", etc.

**✓ Check:** Fuse block mounted and labelled. Correct fuse values in each position.

---

## Step 4 — Mount the 5V PSU

1. Clip the 5V 3A PSU to the right of the fuse block.
2. Label: `5V 3A — LOGIC`

The 5V PSU powers the ESP32-S3 and the relay coils (JD-VCC rail).

**✓ Check:** 5V PSU mounted and labelled. L/N/PE input terminals and +/− output
terminals are accessible.

---

## Step 5 — Mount the 12V PSU

1. Clip the 12V 1A PSU to the right of the 5V PSU.
2. Label: `12V 1A — WATER LEVEL`

The 12V PSU powers the water level sensor and the optional top-off pump.

**✓ Check:** 12V PSU mounted and labelled. Terminals accessible.

---

## Step 6 — Zone Labels on DIN Rail

Apply a strip of red tape or a large label along the mains zone section of the DIN
rail: `HIGH VOLTAGE — UNPLUG BEFORE TOUCHING`

Apply green tape or a label along the LV zone section: `LOW VOLTAGE ≤5V`

**✓ Check:** Components mounted in correct order. All components labelled. Zone
labels applied to DIN rail.

---

## Chapter 03 Checkpoint

- [ ] GFCI/RCD mounted as leftmost (first) component on DIN rail
- [ ] Input fuse holder mounted with 5A fuse installed
- [ ] Blade fuse block mounted with correct per-load fuse ratings
- [ ] 5V 3A PSU mounted and labelled
- [ ] 12V 1A PSU mounted and labelled
- [ ] All components labelled before wiring begins
- [ ] DIN rail zones labelled HIGH VOLTAGE / LOW VOLTAGE
- [ ] No wiring done yet

---

[← Ch 02 — Enclosure Preparation](02-enclosure-prep.md)  ·  [Ch 04 — Relay Module Setup →](04-relay-module-setup.md)
