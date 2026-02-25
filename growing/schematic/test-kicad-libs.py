"""
test-kicad-libs.py — Verify KiCad bundled SKIDL library access

Loads parts from SKiDL's bundled SKIDL-format libraries (which are pre-converted
from KiCad symbols) and wires a small representative circuit:

  3.3V regulator input protection:
    +5V → R (current limit) → D_Zener (5.1V clamp to GND) → C_Polarized (bulk cap)
    → 3.3V_REG → C (bypass cap) → GND

Also verifies all project-relevant IC parts load correctly.

Run:
    python3 test-kicad-libs.py
Output:
    output/test-kicad-libs.svg
"""

import os
import json
import subprocess
import time
from skidl import *

reset()  # clear any state from previous runs in same session


# ─── Verify: passives ────────────────────────────────────────────────────────

print("=== Passives ===")
for lib, name in [
    ('Device', 'R'),
    ('Device', 'C'),
    ('Device', 'C_Polarized'),
    ('Device', 'D_Zener'),
    ('Device', 'D_Schottky'),
    ('Device', 'D_TVS'),
    ('Device', 'D'),
    ('Device', 'LED'),
    ('Device', 'L'),
    ('Device', 'Fuse'),
]:
    p = Part(lib, name, tool=SKIDL, dest=TEMPLATE)
    pins = [(pin.num, pin.name) for pin in p.pins]
    print(f"  {lib}/{name}: ref_prefix={p.ref_prefix!r}  pins={pins}")

print()


# ─── Verify: project ICs ─────────────────────────────────────────────────────

print("=== Project ICs ===")
ic_checks = [
    ('MCU_Espressif',      'ESP32-S3'),
    ('Interface_Expansion','TCA9548APWR'),      # I2C 1-to-8 mux
    ('Sensor_Humidity',    'SHT4x'),            # humidity + temp
    ('Sensor_Gas',         'SCD40-D-R2'),       # CO2 (SCD30 not in bundled libs)
    ('Sensor_Temperature', 'DS18B20U'),         # 1-Wire temp
    ('Sensor_Optical',     'AS7341DLG'),        # light spectrum
]
for lib, name in ic_checks:
    p = Part(lib, name, tool=SKIDL, dest=TEMPLATE)
    pin_names = [pin.name for pin in p.pins]
    print(f"  {lib}/{name}: {len(pin_names)} pins — {pin_names[:6]}{'...' if len(pin_names)>6 else ''}")

print()


# ─── Verify: power symbols ───────────────────────────────────────────────────

print("=== Power symbols ===")
for sym in ['+5V', '+3.3V', '+12V', 'GND', 'PWR_FLAG']:
    p = Part('power', sym, tool=SKIDL, dest=TEMPLATE)
    print(f"  power/{sym}: ref_prefix={p.ref_prefix!r}")

print()


# ─── Small wired circuit ──────────────────────────────────────────────────────
# Schematic: 5V input → R1 (1k series) → D1 (5.1V zener clamp) → C1 (10uF bulk)
# → C2 (100nF bypass) → sensor SDA/SCL pullups (R2, R3) → SHT4x

R       = Part('Device', 'R',          tool=SKIDL, dest=TEMPLATE)
C       = Part('Device', 'C',          tool=SKIDL, dest=TEMPLATE)
Cpol    = Part('Device', 'C_Polarized',tool=SKIDL, dest=TEMPLATE)
DZener  = Part('Device', 'D_Zener',    tool=SKIDL, dest=TEMPLATE)
SHT4x_T = Part('Sensor_Humidity', 'SHT4x', tool=SKIDL, dest=TEMPLATE)

# Power templates
V5    = Part('power', '+5V',   tool=SKIDL, dest=TEMPLATE)
V33   = Part('power', '+3.3V', tool=SKIDL, dest=TEMPLATE)
GND_T = Part('power', 'GND',   tool=SKIDL, dest=TEMPLATE)

# Instantiate
r1   = R(value='1k',    footprint='Resistor_SMD:R_0402')
r2   = R(value='4.7k',  footprint='Resistor_SMD:R_0402')   # SDA pullup
r3   = R(value='4.7k',  footprint='Resistor_SMD:R_0402')   # SCL pullup
d1   = DZener(value='5.1V BZX84')
c1   = Cpol(value='10uF/10V')
c2   = C(value='100nF')
u1   = SHT4x_T()

v5_sym  = V5()
v33_sym = V33()
gnd1    = GND_T()
gnd2    = GND_T()
gnd3    = GND_T()

# Nets
n5v  = Net('+5V')
n33  = Net('+3.3V')
gnd  = Net('GND')
sda  = Net('I2C_SDA')
scl  = Net('I2C_SCL')

# Power symbols → nets
v5_sym['~']  += n5v
v33_sym['~'] += n33
gnd1['~']    += gnd
gnd2['~']    += gnd
gnd3['~']    += gnd

# 5V → R1 → node_a → D1 (zener K to node_a, A to GND) + C1 bulk cap
r1[1] += n5v
r1[2] += n33

# Zener clamp: K → n33 (clamps it), A → GND
d1['K'] += n33
d1['A'] += gnd

# Bulk and bypass caps
c1[1] += n33;  c1[2] += gnd
c2[1] += n33;  c2[2] += gnd

# I2C pullups to 3.3V
r2[1] += n33;  r2[2] += sda
r3[1] += n33;  r3[2] += scl

# SHT4x sensor
u1['VDD'] += n33
u1['VSS'] += gnd
u1['SDA'] += sda
u1['SCL'] += scl


# ─── Generate SVG ─────────────────────────────────────────────────────────────

os.makedirs('output', exist_ok=True)
outfile = 'output/test-kicad-libs'

generate_svg(file_=outfile)
time.sleep(0.5)

# Post-process JSON to show pin names (same as sample.py)
json_path = outfile + '.json'
skin_path = outfile + '_skin.svg'
svg_path  = outfile + '.svg'

pin_map  = {p.ref: {str(pin.num): pin.name for pin in p}
            for p in default_circuit.parts}

with open(json_path) as f:
    data = json.load(f)

for mod in data['modules'].values():
    for ref, cell in mod['cells'].items():
        nm = pin_map.get(ref, {})
        cell['connections']     = {nm.get(k, k): v for k, v in cell['connections'].items()}
        cell['port_directions'] = {nm.get(k, k): v for k, v in cell['port_directions'].items()}
        cell['attributes']['ref'] = ref
        t = cell['type']
        if t.endswith('_1_'):
            t = t[:-3]
        cell['type'] = t

clean_json = outfile + '_clean.json'
with open(clean_json, 'w') as f:
    json.dump(data, f, indent=2)

result = subprocess.run(
    ['netlistsvg', clean_json, '--skin', skin_path, '-o', svg_path],
    capture_output=True, text=True
)
if result.returncode != 0:
    print('netlistsvg error:', result.stderr[:400])
else:
    print(f'Saved: {svg_path}')
