# Reference Build: Martha 2.0 (dccrens, r/MushroomGrowers, Jan 2022)

Source: https://www.reddit.com/r/MushroomGrowers/comments/sbnlib/gourmet_marth_2_dot_o_is_up_and_running_much/
Author: u/dccrens (Northern Virginia, US)
This is version 2 of their build — v1 had rust issues so all metal components were repainted before reassembly.
Post date: January 24, 2022. Score: 274 upvotes. All build details sourced directly from post comments.

---

## Overview

A fully automated Martha tent fruiting chamber with:
- CO2-controlled FAE (fresh air exchange) via inline duct fans
- Humidity-controlled ultrasonic fogger in an external reservoir tub
- Air flows top-right in → bottom-left out (cross-flow)
- Everything modular for easy teardown and cleaning

---

## Parts List

### 1. Fruiting Chamber Tent (Martha)
| Item | Detail |
|------|--------|
| **Greenhouse shelving tent** | Amazon ASIN B07TSC1MB3 |
| Price | $63.99 |
| Notes | Painted all metal poles and shelves with 3 coats Rustoleum (hunter green) *before* assembly to prevent rust. Paint new, not after rusting. |
| Capacity | 3 blocks per shelf (~8 lb each), no issues with weight |

### 2. CO2 Controller
| Item | Detail |
|------|--------|
| **CO2 controller** | Amazon ASIN B08HQMBQ79, $161.01 |
| Mode | "Human mode" with altitude set |
| Controls | Exhaust inline fan + intake computer fan (both simultaneously) |
| Settings | Center=800, Zone=300 → **On at 950 ppm, Off at 650 ppm** |
| Formula | On = Center + (Zone/2) ; Off = Center − (Zone/2) |
| Fan runtime per cycle | ~1–2 minutes |
| Notes | Ran 1 year on a simple timer + cheap CO2 meter before buying controller. Timer is a valid lower-cost approach. |

### 3. Humidity Controller (Primary)
| Item | Detail |
|------|--------|
| **Inkbird IHC200** Humidistat, Dual Stage, Pre-wired Outlet | Amazon ASIN B07XGQ7Q53, $41.99 |
| Controls | Fogger + tub fan (both on same outlet) |
| Set point | 85% RH, 2% offset |
| Actual RH range | 80–95%, averages ~87% |
| Notes | When CO2 exhaust kicks on and RH drops, the IHC200 triggers fogger almost immediately to compensate |

### 4. Humidity Controller (Secondary — Comparison Only)
| Item | Detail |
|------|--------|
| **Secondary humidity controller** | Amazon ASIN B07HF9W41Y |
| Purpose | Cross-check comparison against IHC200; not controlling anything — just verifying readings match |
| Notes | dccrens ran both to confirm accuracy of primary controller |

### 5. Humidity / Temp Monitor (Data Logger)
| Item | Detail |
|------|--------|
| **Inkbird IBS-TH2 Plus** Smart Thermometer / Hygrometer | Amazon ASIN B095W39L4M, ~$30 |
| Features | External temperature probe, digital display, free iOS/Android app, data logging to phone |
| Purpose | Third independent data point; D/L logs to phone; monitors top, middle, bottom (all within 1% RH of each other) |

### 6. Humidifier / Fogger System
| Item | Detail |
|------|--------|
| **House of Hydro ultrasonic fogger** | 5-disc round unit with float; extra discs included free |
| Vendor | https://thehouseofhydro.com/ |
| Reservoir | 19-gallon black/yellow tub (locking lid, no gasket needed) |
| Water type | **Tap water** (NOT distilled — ultrasonic foggers need minerals in tap water) |
| Water consumption | ~5 gallons every 3–4 days |
| Disc maintenance | Swap/clean when fog volume drops; clean old discs and rotate back in |
| Notes | dccrens used this for 1+ year before Martha 2.0 post without disc replacement |
| Anti-mold treatment | Standard H₂O₂ bottle poured into tank weekly |
| UVC lighting | Aquarium-grade UVC lights inside tank on timer: **1 hr ON / 4 hrs OFF** to kill algae/mold in water |
| Notes (UVC) | 24/7 UVC is not necessary; shortens bulb lifespan |

### 7. Fogger Tub Fan
| Item | Detail |
|------|--------|
| Computer fan on top of reservoir tub | Blows air *down* through tub, forcing humidified fog *up* the 4" pipe into the tent |
| Controlled by | Inkbird IHC200 (same outlet as fogger) |

### 8. Exhaust Fan (FAE Out)
| Item | Detail |
|------|--------|
| **AC Infinity CLOUDLINE S4** | Quiet 4" Inline Duct Fan with Speed Controller |
| Amazon listing | "Ventilation Exhaust Fan for Heating Cooling Booster, Grow Tents, Hydroponics" |
| Price | $99 |
| Controlled by | CO2 controller |
| Placement | Bottom-left of tent, vented via flex duct through ceiling into home HVAC ductwork → outside |
| Notes | OP bought **two** of these — swaps them for cleaning without interrupting the grow |

### 9. Intake Fan (FAE In)
| Item | Detail |
|------|--------|
| **4" waterproof computer fan** | Pushes fresh room air into tent |
| Placement | Top-right of tent |
| Controlled by | CO2 controller (turns on simultaneously with exhaust fan) |
| Notes | Pulls FAE from the room the Martha is in |

### 10. Ducting & Plumbing
| Item | Detail |
|------|--------|
| 4" flexible silver duct (flexi pipe) | Runs from fogger tub up into tent (right side) and from tent out to house ductwork (left side) |
| 4" galvanized adjustable elbow | Top of fogger flex pipe; allows slight upward angle into tent so condensation drains back to tub |
| Custom inlet adapter | Flat hard plastic piece screwed to Martha frame, 4" hole — elbow press-fits in (tight enough to hold) |
| Imgur reference photo | https://imgur.com/a/vnFbMfp |

### 11. Drip Tray / Floor
| Item | Detail |
|------|--------|
| **Commercial drip tray** | Amazon ASIN B0002NNWDW, 41.3 × 27.75 × 1 inches |
| Custom liner | 6-mil plastic sheet, folded with 1" lip at corners, velcroed inside tent above bottom wire shelf |
| Perlite | **Not used** — OP considers it a mold reservoir |

### 12. Lighting
| Item | Detail |
|------|--------|
| LED strips (upgraded in v2) | Better LED lighting vs v1; mounted inside tent |
| UVC lights | Aquarium-grade, inside humidity reservoir only (not in tent) |

### 13. Consumables / Misc
| Item | Detail |
|------|--------|
| Rustoleum spray paint | Hunter green; 3 coats on all metal tent components |
| Hydrogen peroxide (H₂O₂) | Standard pharmacy bottle; weekly into fogger tank + bimonthly wipe-down of entire Martha |
| Velcro | Used extensively instead of glue or zip ties — key to modularity |
| Grow bags / media | Amazon or https://mushroommediaonline.com/ |

---

## System Architecture

```
                    [Room Air]
                        |
              [4" Waterproof Computer Fan]  ← intake FAE
              [Top-Right of Tent]
                        |
                        ↓
    ┌───────────────────────────────────┐
    │         MARTHA TENT               │
    │  Shelf 4  [blocks]                │
    │  Shelf 3  [blocks]                │
    │  Shelf 2  [blocks]                │
    │  Shelf 1  [blocks]                │
    │  [Drip Tray + 6mil plastic liner] │
    └──────────────────┬────────────────┘
                       │ CO2-rich air exits
                       ↓ bottom-left
              [AC Infinity S4 Inline Fan]
                       |
              [Flex duct → ceiling duct → outside]

    Humidity side (right):
    [19-gal tub w/ HoH 5-disc fogger]
    [UVC lights on 1hr/4hr timer]
    [Tub fan blows down → fog rises]
    [4" flex pipe up → elbow → tent top-right]
    Controlled by: Inkbird IHC200

    CO2/FAE control:
    Both intake fan + exhaust fan triggered simultaneously by CO2 controller
    On: 950 ppm / Off: 650 ppm / Cycle: ~1-2 min
```

---

## Environmental Targets (Gourmet Mushrooms)

| Parameter | Target | Range | Notes |
|-----------|--------|-------|-------|
| RH | 85% | 80–95% | Averages 87% in practice |
| CO2 | 800 ppm center | 650–950 ppm | On at 950, off at 650 |
| CO2 (actives) | 700 ppm center | 500–900 ppm | Per OP's guidance |
| CO2 (gourmet) | 600 ppm center | 400–800 ppm | Per OP's guidance |

---

## Cost Summary (circa 2022)

| Component | ASIN | Cost (USD) |
|-----------|------|-----------|
| Martha tent | B07TSC1MB3 | $63.99 |
| CO2 controller | B08HQMBQ79 | $161.01 |
| Inkbird IHC200 humidity controller (primary) | B07XGQ7Q53 | $41.99 |
| Secondary humidity controller (comparison) | B07HF9W41Y | — |
| Inkbird IBS-TH2 Plus monitor | B095W39L4M | ~$30.00 |
| AC Infinity CLOUDLINE S4 (×2) | — | ~$198.00 |
| House of Hydro 5-disc fogger | — | — |
| Commercial drip tray | B0002NNWDW | — |
| Misc (velcro, H₂O₂, paint, fans, ducting) | — | — |
| **Total (rough, per another commenter)** | | **~$250 USD / ~$300 CAD** |

---

## Key Design Lessons from OP

1. **Paint all metal before assembly** — 3 coats Rustoleum. Don't wait until it rusts.
2. **No perlite** — not needed; can harbor mold.
3. **No distributed PVC piping for fog** — fog is heavier than air, enters at top and diffuses uniformly. PVC pipes trap water and mold.
4. **Modular everything** — velcro over zip ties/glue. Easy to break down for cleaning.
5. **Two exhaust fans** — swap for cleaning without stopping the grow.
6. **Slight upward pipe angle for fogger inlet** — condensation drains back to tank, no pooling.
7. **Tap water in fogger** — not distilled; ultrasonic foggers need tap mineral content.
8. **Timer approach is valid** — ran 1 year with timer + cheap CO2 meter before buying the controller. (Use CO2 sensor to observe, then set timer to X min/hr and dial in from there.)
9. **H₂O₂ weekly in tank + UVC timer** — effective 1-2 punch against mold/algae in water.
10. **Cross-flow airflow** — fog/fresh air in top-right, CO2 out bottom-left.
11. **Full Martha cleaning cadence** — bimonthly (~every 2 months): spray everything with H₂O₂ and wipe down. Modular design is critical for this.
12. **CO2 controller note** — dccrens sometimes says "800/600 ppm" colloquially, but the actual programmed settings are Center=800, Zone=300 → **On=950 / Off=650**. The formula is authoritative.

---

## Species Grown (per thread)
- Oysters (multiple varieties, top and side fruiting, 5th flush shown)
- Reishi (top-left shelf, just starting to antler)
- Gourmet varieties in general
