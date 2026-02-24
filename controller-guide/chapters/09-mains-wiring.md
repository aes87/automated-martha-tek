# Chapter 09 — Mains Wiring

**What you'll do:** Connect all line-voltage wiring: mains input through GFCI/RCD
and input fuse, then to both PSUs; relay COM terminals through the per-load fuse
block to each load; shared neutral bus and earth connections.

**Prerequisites:** Chapters 02–08 complete. All mains-zone components mounted and
labelled (Chapter 03). Wago 221 connectors, 14 AWG wire, 18 AWG wire, ferrule
crimp kit, multimeter.

---

## ⚠️ Before You Begin: Mains Safety

> ⚠️ **SAFETY — READ EVERY ITEM BEFORE TOUCHING MAINS ZONE:**
>
> 1. **The enclosure must be unplugged from the wall.** Verify this physically —
>    look at the wall outlet and confirm the mains plug is out.
> 2. **If in doubt, have a licensed electrician do this chapter.** Mains wiring
>    errors cause fires and electrocution. There is no shame in hiring a professional
>    for the mains-side work. Everything in Chapters 02–08 and 10–12 is low-voltage.
> 3. **Never work with one hand on a mains terminal and the other on a grounded
>    object.** If an accidental connection occurs, current would pass through your
>    chest. Work with one hand, keep the other in your pocket or behind your back.
> 4. **All mains connections must use ferrule-terminated wire ends.** No bare wire
>    strands in any screw terminal. Crimp a ferrule on every stripped wire end
>    before it enters any terminal or Wago connector.
> 5. **Use 14 AWG minimum for mains runs** (from wall to GFCI, GFCI to fuse, fuse
>    to PSUs). Use 16–18 AWG for load leads (fans, fogger, lights).
> 6. **Wago 221 connectors** for all mains splices and junctions. Do not use wire
>    nuts or open-backed barrier strips for mains wiring in an enclosed box.

**Standard wire colours (North America):**
- Black: Hot (Line)
- White: Neutral
- Green (or bare): Earth/PE

**Standard wire colours (Europe/international):**
- Brown: Live (Line)
- Blue: Neutral
- Green/Yellow: Earth/PE

Use whichever colour scheme matches your local standard and apply it consistently.

---

## Mains Wiring Order

Wire in this sequence — it matches the physical left-to-right order on the DIN rail
and ensures each section is complete before the next is started:

1. Mains input cable → GFCI/RCD
2. GFCI/RCD → input fuse holder
3. Input fuse holder → PSU inputs (both PSUs)
4. Input fuse holder → relay COM bars
5. Per-load fuses → load cable Live wires
6. Shared neutral bus — all Neutrals
7. Earth / PE bus — all grounds

---

## 9a — Mains Input Cable to GFCI/RCD

1. Thread the mains input cable through the PG16 cable gland from outside. Leave
   enough slack inside the enclosure to reach the GFCI/RCD without tension.
2. Strip 15 mm of outer jacket and 8 mm of insulation from each conductor.
3. Crimp ferrules on all three conductors (Hot, Neutral, Earth).
4. Connect to the GFCI/RCD **LINE** terminals:
   - Black (Hot) → LINE L or LINE terminal
   - White (Neutral) → LINE N terminal
   - Green/bare (Earth) → PE terminal on the GFCI or enclosure chassis

> ⚠️ **SAFETY:** The GFCI/RCD has LINE and LOAD sides — they are different terminals.
> The mains input goes to LINE. If you reverse them (input to LOAD), the GFCI will
> not protect against ground faults. Check the labels on the device before connecting.

**Tighten cable gland** around the mains input cable once the cable is in final
position — the rubber seal must grip the outer jacket.

**✓ Check:** Mains input connected to GFCI LINE terminals. Cable gland tightened.
Hot → L, Neutral → N, Earth → PE. No bare wire visible at any terminal.

---

## 9b — GFCI/RCD LOAD Terminals to Input Fuse Holder

1. Run a short (100–150 mm) 14 AWG Hot wire from the GFCI/RCD LOAD-L terminal
   to one terminal of the input fuse holder.
2. Run a short 14 AWG Neutral wire from the GFCI/RCD LOAD-N terminal to the
   Neutral bus bar (terminal block on DIN rail).

The fuse holder is in-line on the Hot wire only. Neutral passes straight through
to the neutral bus.

> **[?] Why fuse only the Hot wire?** Only the Live (Hot) wire is interrupted in
> a fuse or circuit breaker. This is standard practice. The Neutral and Earth are
> never interrupted because doing so would leave the equipment energised (Neutral
> interrupted) or unprotected (Earth interrupted) even though no current flows.

**✓ Check:** GFCI LOAD-L connected through fuse holder. GFCI LOAD-N connected
to neutral bus bar. Fuse holder is in-line on the Hot wire only.

---

## 9c — Input Fuse Output to PSU Inputs and Relay COM Bar

From the output side of the input fuse holder, the Hot wire feeds two destinations:

**To both PSUs (in parallel):**
1. From fuse holder output, run Hot wire to both PSU L (Line) input terminals
   using Wago 221 3-way connectors: fuse output → Wago → 5V PSU L → 12V PSU L.
2. From the Neutral bus, run Neutral wire to both PSU N terminals: Wago → 5V PSU N
   → 12V PSU N.
3. From the Earth bus, run PE wire to both PSU PE/GND terminals and to the
   enclosure chassis.

**To relay COM bar (for load switching):**
The Hot wire also feeds the relay COM terminals (the source side of each load
circuit). After all fusing, run Hot to the blade fuse block's input bus rail.
From each fuse output, connect to the corresponding relay channel's COM terminal.

Practical approach:
- Wago 221 5-way connector: fuse holder output Hot → one input; four output legs
  to (a) 5V PSU L, (b) 12V PSU L, (c) fuse block input Hot, (d) leave one position
  for a future service connection.

**PSU PE and enclosure chassis earth:**
- Connect Earth from the mains input cable to: 5V PSU PE, 12V PSU PE, and the
  enclosure chassis (use a self-tapping screw into the enclosure metal, with a ring
  terminal on the wire).

**✓ Check:** Both PSUs have L, N, PE connected. Relay fuse block has Hot supply.
Enclosure chassis has Earth connection. All via ferrule-terminated connections.
No bare wire visible.

---

## 9d — Per-Load Fuse Block to Relay COM Terminals

From the blade fuse block, connect each fuse output to the COM terminal of its
corresponding relay channel:

| Fuse position | Load | Fuse rating | Relay COM |
|--------------|------|------------|-----------|
| 1 | Fogger | 2A | Relay Ch1 COM |
| 2 | Tub fan | 2A | Relay Ch2 COM |
| 3 | Exhaust fan | 2A | Relay Ch3 COM |
| 4 | Intake fan | 2A | Relay Ch4 COM |
| 5 | UVC lights | 2A | Relay Ch5 COM |
| 6 | Grow lights | 3A | Relay Ch6 COM |

Use 18 AWG wire (16 AWG preferred) for these runs. Crimp ferrules on all ends.

> ⚠️ **SAFETY:** The relay board's load terminals are now at mains voltage once
> wired. Keep the enclosure unplugged until the full mains wiring section (9a–9f)
> is complete and all checkboxes in Chapter 10 are ticked. Do not power on with
> partial mains wiring.

---

## 9e — Load Output Wiring (Relay NO to Load Cables)

Each load (fogger, fans, UVC, grow lights) has a cable that enters through a PG11
cable gland. For each load:

1. Thread the load cable through its PG11 gland from outside.
2. Strip the Live (Hot) wire and connect it to the relay Normally Open (NO) terminal
   for that channel.
3. Strip the Neutral wire and connect it to the Neutral bus bar.
4. If the load has an Earth wire (earthed loads: fans, UVC fixture), connect it to
   the Earth bus.

**Connection summary for each load:**

```
Load cable Hot (Black/Brown) ────── Relay Channel NO terminal
Load cable Neutral (White/Blue) ─── Neutral bus bar
Load cable Earth (Green/bare) ────── Earth bus (if earthed load)
```

5. Tighten each cable gland around the load cable jacket once the cable is in final
   position.

> **[?] Normally Open (NO) vs Normally Closed (NC):** "Normally Open" means the
> contact is open (circuit off) when the relay is not energised. When the relay
> fires, it closes the contact — completing the circuit and powering the load. You
> always use NO for loads you want to switch on. NC would do the opposite: load
> on when relay is off.

**✓ Check:** All load cable Hot wires connected to relay NO terminals. All load
Neutrals connected to Neutral bus. All earthed loads connected to Earth bus.
Cable glands tightened.

---

## 9f — Neutral Bus and Earth Bus

By now, all neutral and earth connections should be landing on terminal blocks:

**Neutral bus (shared neutral):**
- GFCI LOAD-N
- 5V PSU N
- 12V PSU N
- Load cable Neutrals (all 6 or more loads)

All of these connect to the same neutral bus terminal block on the DIN rail.
The Neutral wire from the mains input (from the wall) reaches all loads by sharing
this bus.

**Earth/PE bus:**
- Mains input Earth
- 5V PSU PE
- 12V PSU PE
- Enclosure chassis (ring terminal on screw)
- Load cable Earths (fans, UVC, any other earthed load)

All of these connect to the Earth bus terminal block or directly to the enclosure
chassis PE screw.

**Final earth continuity check:**
1. With the enclosure still unplugged, set multimeter to continuity mode.
2. One probe on the mains input Earth wire (at the GFCI/RCD PE terminal).
3. Other probe on the enclosure chassis metal.
4. Should show continuity (near-zero resistance).
5. Move the second probe to each PSU PE terminal — all should show continuity.

**✓ Check:** Earth continuity from mains input through to enclosure chassis and
both PSU PE terminals confirmed.

---

## Chapter 09 — Full Mains Wiring Checkpoint

Before the enclosure lid is closed, do a complete visual inspection of the mains zone:

**Connections:**
- [ ] Mains input: Hot → GFCI LINE-L; Neutral → GFCI LINE-N; Earth → PE terminal
- [ ] GFCI LOAD-L → input fuse holder → Hot bus
- [ ] GFCI LOAD-N → Neutral bus
- [ ] Hot bus → 5V PSU L and 12V PSU L (Wago 221 connector)
- [ ] Neutral bus → 5V PSU N and 12V PSU N
- [ ] Earth bus → 5V PSU PE, 12V PSU PE, enclosure chassis
- [ ] Hot bus → blade fuse block input bus
- [ ] Each fuse block output → relay COM terminal (correct fuse per channel)
- [ ] Each relay NO terminal → load cable Hot wire
- [ ] All load Neutrals → Neutral bus
- [ ] All earthed load cables → Earth bus
- [ ] Mains input cable gland tightened
- [ ] All load cable glands tightened

**Wire quality:**
- [ ] Ferrule crimped on every wire end in every terminal — no bare strands visible
- [ ] All Wago 221 levers fully closed (lever down = wire secured)
- [ ] No wire insulation stripped back more than 10 mm at any terminal
- [ ] 14 AWG used for mains input runs; 16–18 AWG for load leads

**Mechanical:**
- [ ] No loose wire ends anywhere in the mains zone
- [ ] All relay board load terminals (COM/NO) are in the mains zone only
- [ ] No LV wiring (sensor cables, ESP32 signal wires) runs through the mains zone

> ⚠️ **SAFETY:** Complete every item on this list before closing the lid. If you
> cannot tick a checkbox, do not power on. Resolve the issue first.
