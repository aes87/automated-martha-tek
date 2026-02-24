# SKiDL Schematic Reference — Martha Controller

How the schematic pipeline works, what libraries are available, and how to
swap in a different renderer without rewriting the netlist.

---

## How It Works

```
generate.py  ──►  SKiDL ERC  ──►  generate_svg()  ──►  *.json + *_skin.svg
                                                               │
                                                         netlistsvg
                                                               │
                                                          *.svg  ◄── output
```

SKiDL describes the circuit as Python objects (Parts + Nets), runs an ERC
check, then serialises to a Yosys-style JSON netlist. `netlistsvg` reads that
JSON and renders the SVG. The `_skin.svg` controls symbol shapes; the
`_clean.json` is the post-processed netlist with human-readable pin names.

---

## Library Sources

SKiDL ships **226 SKIDL-format libraries** (`.py` files) pre-converted from the
official KiCad symbol library. These live at:

```
~/.local/lib/python3.11/site-packages/skidl/tools/skidl/libs/
```

They are already in `lib_search_paths[SKIDL]` — no environment variables needed.

### Using a library part

```python
from skidl import *

# Template (blueprint) — not yet placed in circuit
R = Part('Device', 'R', tool=SKIDL, dest=TEMPLATE)

# Instantiate into circuit
r1 = R(value='4.7k', ref='R1')
r1[1] += net_a
r1[2] += net_b
```

### If KiCad is installed (not this container)

```python
# Set the path to KiCad's .kicad_sym files, then use tool=KICAD
import os
os.environ['KICAD_SYMBOL_DIR'] = '/usr/share/kicad/symbols'

# OR programmatically:
lib_search_paths[KICAD].append('/path/to/kicad-symbols')
set_default_tool(KICAD)

R = Part('Device', 'R', tool=KICAD, dest=TEMPLATE)
```

---

## Parts Used in This Project

### From bundled SKIDL libraries

| Ref | Library | Part name | Key pins |
|-----|---------|-----------|----------|
| U_MCU | `MCU_Espressif` | `ESP32-S3` | GPIO0-46, MTCK/MTDO/MTDI/MTMS, VDD3P3, GND |
| U_MUX | `Interface_Expansion` | `TCA9548APWR` | SDA, SCL, SD0-7, SC0-7, A0-2, VCC, GND, ~RESET |
| U_SHT | `Sensor_Humidity` | `SHT4x` | SDA, SCL, VDD, VSS |
| U_AS7341 | `Sensor_Optical` | `AS7341DLG` | SDA, SCL, VDD, GND, PGND, LDR, INT, GPIO |
| U_DS | `Sensor_Temperature` | `DS18B20U` | DQ, V_{DD}, GND, NC×5 |
| R | `Device` | `R` | pins 1, 2 (unnamed `~`) |
| C | `Device` | `C` | pins 1, 2 |
| C_pol | `Device` | `C_Polarized` | pins 1, 2 |
| D_Z | `Device` | `D_Zener` | K, A |
| D_Sch | `Device` | `D_Schottky` | K, A |
| D_TVS | `Device` | `D_TVS` | A1, A2 |
| Fuse | `Device` | `Fuse` | pins 1, 2 |
| J | `Connector_Generic` | `Conn_01x02` … `Conn_01x08` | Pin_1 … Pin_N |

Power symbols (`tool=SKIDL`, `lib='power'`): `+5V`, `+3.3V`, `+12V`, `GND`, `PWR_FLAG`

### Custom parts (defined in `custom_parts.py`)

| Name | Description | Pins |
|------|-------------|------|
| `GFCI_RCD` | 30mA GFCI/RCD module | L_IN, N_IN, PE_IN, L_OUT, N_OUT, PE_OUT |
| `PSU_5V3A` | Meanwell HDR-30-5 | L, N, PE, +5V_OUT, GND_OUT |
| `PSU_12V1A` | Meanwell HDR-15-12 | L, N, PE, +12V_OUT, GND_OUT |
| `Relay_8ch` | 8-ch PC817 relay board (active LOW) | VCC, GND, JD_VCC, IN1-8, COM1-8, NO1-8, NC1-8 |
| `FailsafePanel` | DPDT master + 4× SPST group switches | ESP_CH1-8, OUT_CH1-8, VCC, GND |
| `SCD30_Breakout` | Adafruit SCD-30 CO2 breakout (I2C 0x61) | VIN, 3Vo, GND, SCL, SDA, SEL |
| `SCD40_Breakout` | Adafruit SCD-40 CO2 breakout (I2C 0x62) | VIN, 3Vo, GND, SCL, SDA |
| `Water_Level_Sensor` | 4-20 mA submersible pressure sensor | VIN, IOUT |

---

## ESP32-S3 Pin Name Map

The KiCad library uses IC-package pin names, not GPIO numbers. Mapping for the
pins used in this build:

| Firmware GPIO | KiCad pin name | Function |
|---------------|----------------|----------|
| GPIO38 | `GPIO38` | Relay Ch1 Fogger |
| GPIO39 | `MTCK` | Relay Ch2 Tub fan (JTAG TCK dual-function) |
| GPIO18 | `GPIO18` | Relay Ch3 Exhaust |
| GPIO19 | `GPIO19/USB_D-` | Relay Ch4 Intake (USB D- dual-function; not USB here) |
| GPIO40 | `MTDO` | Relay Ch5 UVC |
| GPIO41 | `MTDI` | Relay Ch6 Lights |
| GPIO42 | `MTMS` | Relay Ch7 Pump |
| GPIO46 | `GPIO46` | Relay Ch8 Spare (firmware uses GPIO47; not in bare-die symbol) |
| GPIO21 | `GPIO21` | I2C SDA |
| GPIO9 | `GPIO9` | I2C SCL |
| GPIO4 | `GPIO4` | 1-Wire DS18B20 bus |
| GPIO7 | `GPIO7` | ADC water level (ADC1_CH6) |

**Note on GPIO47:** The KiCad ESP32-S3 bare-die symbol does not expose GPIO47
(used in firmware as spare relay). It maps to the WROOM-1 module header pin,
not a named pad in the QFN56 package symbol. GPIO46 is substituted in the
schematic. To fix properly, define a custom `ESP32S3_DevKitC1` module part
that lists the actual 38-pin header instead of the chip package.

---

## Rendering Approaches

The `render()` function in `generate.py` uses netlistsvg. To try alternatives,
replace or augment the render step — the `_clean.json` file is always written
first and can be consumed by any Yosys-compatible viewer.

### Current: netlistsvg (default)

```python
subprocess.run(['netlistsvg', clean_json, '--skin', skin_path, '-o', svg_path])
```

- Output: SVG with boxes and labelled wires
- Skin: `*_skin.svg` controls symbol shapes (default from SKiDL)
- Customise skin: copy `*_skin.svg`, edit SVG `<symbol>` elements, pass `--skin`

```bash
# View available netlistsvg options
netlistsvg --help

# Try a different skin
netlistsvg output/martha-controller_clean.json \
    --skin my_skin.svg \
    -o output/martha-controller-custom.svg
```

### Alternative: Yosys netlist viewer

`_clean.json` is valid Yosys netlist JSON. Any Yosys-compatible tool can read it:

```bash
# Yosys show (opens graphviz window — needs display)
yosys -p "read_json output/martha-controller_clean.json; show"
```

### Alternative: schemdraw (manual layout)

schemdraw draws individual elements with explicit x/y positioning — better for
a final polished schematic but requires manually laying out each symbol.
See `test-power.py` for a schemdraw example.

```python
import schemdraw
import schemdraw.elements as elm

with schemdraw.Drawing() as d:
    d += elm.Resistor().label('4.7kΩ')
    d += elm.Capacitor().down()
    # ...
d.save('output/sheet1.svg')
```

### Alternative: KiCad schematic (requires KiCad install)

```python
# Requires kicad-symbols cloned/installed and KICAD_SYMBOL_DIR set
lib_search_paths[KICAD].append('/path/to/kicad-symbols')
set_default_tool(KICAD)

# Replace tool=SKIDL with tool=KICAD on Part() calls, then:
generate_schematic()    # produces .sch openable in KiCad Eeschema
```

### Alternative: graphviz dot export

```python
# SKiDL can write a dot file for graphviz
generate_dot(file_='output/martha-controller')
# Then: dot -Tsvg output/martha-controller.dot -o output/martha-controller-dot.svg
```

---

## ERC Noise Reference

These warnings are expected on this schematic and safe to ignore:

| Warning | Cause | Safe to ignore? |
|---------|-------|-----------------|
| `Insufficient drive current on AC_L/AC_N/AC_PE` | Mains nets have no PWROUT symbol driving them | Yes — add `PWR_FLAG` symbols to suppress |
| `Insufficient drive current on GND` | PSU GND pins are PASSIVE, not PWROUT | Yes — cosmetic; GND is driven by power symbols |
| `Insufficient drive current on L_FUSED` | Same as AC nets | Yes |
| `Unconnected pin: ... of ESP32-S3/U4` | Unused GPIOs on bare-die symbol | Yes — NC expected on a partial-use IC |
| `NC pin ... of DS18B20U` | Guard-ring pins on TO-92 package | Yes — intentional NC |

To suppress AC drive warnings, add `PWR_FLAG` symbols to `AC_L`, `AC_N`, `AC_PE`:

```python
pf = Part('power', 'PWR_FLAG', tool=SKIDL, dest=TEMPLATE)
pf()['~'] += ac_l
pf()['~'] += ac_n
pf()['~'] += ac_pe
```

---

## Project Files

```
schematic/
├── generate.py          # Full schematic — 4 sections, run to regenerate
├── custom_parts.py      # Custom part definitions (PSUs, relay board, sensors)
├── test-kicad-libs.py   # Library verification + simple wired example
├── test-power.py        # Sheet 1 only (schemdraw, legacy)
├── sample.py            # Original sample circuit (SKIDL-tool parts, manual)
├── SKIDL-REFERENCE.md   # This file
└── output/
    ├── martha-controller.svg        # Full schematic render
    ├── martha-controller_clean.json # Post-processed Yosys netlist
    ├── martha-controller_skin.svg   # netlistsvg skin (edit to customise symbols)
    └── ...
```
