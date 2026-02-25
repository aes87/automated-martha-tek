# Schematic Subproject

Generates SVG schematics for the DIY ESP32-S3 Martha tent controller using schemdraw.

## Quick Start

```bash
pip install -r requirements.txt          # one-time setup
python3 generate.py                      # generate all sheets
python3 test-power.py                    # regenerate Sheet 1 test only
```

## Files

| File | Purpose |
|------|---------|
| `generate.py` | Full 4-sheet schematic (not yet written) |
| `test-power.py` | Sheet 1 (Power Supply) — test render |
| `circuit-spec.yaml` | Source-of-truth netlist/component data (not yet written) |
| `output/` | Generated SVG files |

## Architecture

All scripts use `schemdraw` (SVG backend, no GUI required).

### Coordinate conventions
- `unit=3.5` — default element length
- schemdraw y increases **upward** (math convention); `down(n)` decreases y by n
- IC pin auto-spacing: first pin = top anchor (highest y), subsequent pins descend
- `anchor('PinName')` places that pin at the `.at()` coordinate

### IC box helper (`ic_box`)
```python
ic_box(label, left_pins, right_pins, pinspacing=0.85)
```
- Auto-spaces pins; do **not** set `pos=` (pos=0.0 is falsy and breaks placement)
- The first left pin becomes the top-left anchor
- Use `.anchor('PinName')` when placing with `.at()`

### Computing vertical offsets
```python
# Place element N units below a reference point:
target_y = ref_y - N * d.unit

# Wire drop from ref down to target:
elm.Line().at(ref_pos).down(ref_pos[1] - target_pos[1])
```

## Planned Sheets

1. **Power** — AC Mains → GFCI/RCD → 5A fuse → 5V/3A PSU + 12V/1A PSU
2. **Controller Core** — ESP32-S3 DevKitC-1, 8-ch relay module, failsafe panel
3. **Sensor Bus** — TCA9548A → SHT45×3, SCD30, AS7341; DS18B20 1-Wire; ADC protection
4. **Loads** — Relay contacts, per-load blade fuses, mains loads

## Hardware Reference

See `/tmp/automated-martha-tek/diy-controller-build-guide.md` for full circuit spec
and `/tmp/automated-martha-tek/diy-controller-hardware-reference.md` for component rationale.
