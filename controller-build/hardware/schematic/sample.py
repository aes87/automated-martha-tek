"""
sample.py — SKiDL sample circuit for visual evaluation

A representative slice of the full Martha controller:
  • Power supply:  GFCI → 5A fuse → 5V PSU + 12V PSU
  • MCU:           ESP32-S3 (subset of pins)
  • Relay module:  8-ch opto-isolated, 2 channels wired up
  • Sensor:        SCD30 CO₂ sensor on I2C

Run:
    python3 sample.py
Output:
    output/sample.svg
"""

import json, os, subprocess, time
from skidl import *

# ── helpers ──────────────────────────────────────────────────────────────────

def part(ref_prefix, name, desc, pins_left, pins_right=None, pins_nc=None):
    """Define a SKIDL-tool part with named pins on left/right/nc sides."""
    p = Part(name=name, ref_prefix=ref_prefix, tool=SKIDL,
             dest=TEMPLATE, description=desc)
    num = 1
    for pname in (pins_left or []):
        p += Pin(num=str(num), name=pname, func=Pin.types.PWRIN);  num += 1
    for pname in (pins_right or []):
        p += Pin(num=str(num), name=pname, func=Pin.types.PWROUT); num += 1
    for pname in (pins_nc or []):
        p += Pin(num=str(num), name=pname, func=Pin.types.NOCONNECT); num += 1
    return p


def generate(outfile):
    """Run SKiDL generate_svg, post-process JSON for pin names, re-run netlistsvg."""
    # Build pin-number → name map and part-name map before generating
    pin_map  = {p.ref: {str(pin.num): pin.name for pin in p}
                for p in default_circuit.parts}
    ref_name = {p.ref: p.name for p in default_circuit.parts}

    generate_svg(file_=outfile)
    time.sleep(0.3)   # generate_svg fires netlistsvg asynchronously

    json_path = outfile + '.json'
    skin_path = outfile + '_skin.svg'
    svg_path  = outfile + '.svg'

    with open(json_path) as f:
        data = json.load(f)

    for mod in data['modules'].values():
        for ref, cell in mod['cells'].items():
            nm = pin_map.get(ref, {})
            cell['connections']     = {nm.get(k, k): v
                                        for k, v in cell['connections'].items()}
            cell['port_directions'] = {nm.get(k, k): v
                                        for k, v in cell['port_directions'].items()}
            # Show "U1" reference label instead of ugly "PART_NAME_1_"
            cell['attributes']['ref'] = ref
            # Clean "_1_" suffix from type name
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


# ═══════════════════════════════════════════════════════════════════════════
# Part templates
# ═══════════════════════════════════════════════════════════════════════════

GFCI   = part('U', 'GFCI_RCD',   'GFCI/RCD 30mA',
               pins_left  = ['L_IN', 'N_IN', 'PE_IN'],
               pins_right = ['L_OUT', 'N_OUT', 'PE_OUT'])

FUSE   = part('F', 'FUSE_5A',    '5A panel fuse',
               pins_left  = ['IN'],
               pins_right = ['OUT'])

PSU5   = part('U', 'PSU_5V3A',   '5V/3A DIN PSU (HDR-15-5)',
               pins_left  = ['L', 'N', 'PE'],
               pins_right = ['+5V_OUT', 'GND_OUT'])

PSU12  = part('U', 'PSU_12V1A',  '12V/1A DIN PSU',
               pins_left  = ['L', 'N', 'PE'],
               pins_right = ['+12V_OUT', 'GND_OUT'])

# ESP32-S3 DevKitC-1 — just the pins used in this sample
ESP32  = part('U', 'ESP32S3',    'ESP32-S3-DevKitC-1 (38-pin)',
               pins_left  = ['3V3_IN', 'GND', 'GPIO21_SDA', 'GPIO9_SCL',
                              'GPIO38_REL1', 'GPIO39_REL2'],
               pins_right = [])

RELAY  = part('U', 'RELAY_8CH',  '8-ch opto-isolated relay module',
               pins_left  = ['VCC', 'GND', 'JD_VCC',
                              'IN1', 'IN2'],
               pins_right = ['COM1', 'NO1', 'NC1',
                              'COM2', 'NO2', 'NC2'])

SCD30  = part('U', 'SCD30',      'Sensirion SCD30 CO2/RH/T sensor (I2C 0x61)',
               pins_left  = ['VDD', 'GND', 'SCL', 'SDA', 'SEL'],
               pins_right = ['TX', 'RX'])

# Loads (simplified — just the fogger)
FOGGER = part('J', 'FOGGER',     'Ultrasonic fogger (120VAC)',
               pins_left  = ['L_IN', 'N_IN'])


# ═══════════════════════════════════════════════════════════════════════════
# Nets
# ═══════════════════════════════════════════════════════════════════════════

ac_l     = Net('AC_L')
ac_n     = Net('AC_N')
ac_pe    = Net('AC_PE')
l_fused  = Net('L_FUSED')
v5       = Net('+5V')
v12      = Net('+12V')
gnd      = Net('GND')
sda      = Net('I2C_SDA')
scl      = Net('I2C_SCL')
rel1_sig = Net('REL1_SIG')
rel2_sig = Net('REL2_SIG')
fogger_l = Net('FOGGER_L')
fogger_n = Net('FOGGER_N')


# ═══════════════════════════════════════════════════════════════════════════
# Instantiate and connect
# ═══════════════════════════════════════════════════════════════════════════

gfci  = GFCI()
fuse  = FUSE()
psu5  = PSU5()
psu12 = PSU12()
mcu   = ESP32()
relay = RELAY()
scd30 = SCD30()
fog   = FOGGER()

# ── Mains path ───────────────────────────────────────────────────────────────
gfci['L_IN']  += ac_l
gfci['N_IN']  += ac_n
gfci['PE_IN'] += ac_pe

fuse['IN']  += gfci['L_OUT']
fuse['OUT'] += l_fused

psu5['L']  += l_fused;  psu5['N']  += gfci['N_OUT'];  psu5['PE']  += gfci['PE_OUT']
psu12['L'] += l_fused;  psu12['N'] += gfci['N_OUT'];  psu12['PE'] += gfci['PE_OUT']

psu5['+5V_OUT']  += v5;   psu5['GND_OUT']  += gnd
psu12['+12V_OUT'] += v12; psu12['GND_OUT'] += gnd

# ── MCU power + I2C ──────────────────────────────────────────────────────────
mcu['3V3_IN']     += v5       # 3.3V reg on DevKitC powered from 5V USB pin
mcu['GND']        += gnd
mcu['GPIO21_SDA'] += sda
mcu['GPIO9_SCL']  += scl
mcu['GPIO38_REL1']+= rel1_sig
mcu['GPIO39_REL2']+= rel2_sig

# ── Relay module ─────────────────────────────────────────────────────────────
relay['VCC']    += v5
relay['GND']    += gnd
relay['JD_VCC'] += v5      # jumper removed in real build; shown here for clarity
relay['IN1']    += rel1_sig
relay['IN2']    += rel2_sig

# ── SCD30 sensor ─────────────────────────────────────────────────────────────
scd30['VDD'] += v5
scd30['GND'] += gnd
scd30['SDA'] += sda
scd30['SCL'] += scl
scd30['SEL'] += gnd    # I2C mode (SEL pulled low)

# ── Relay output → fogger load ───────────────────────────────────────────────
relay['COM1'] += l_fused    # switched L
relay['NO1']  += fogger_l
fog['L_IN']   += fogger_l
fog['N_IN']   += gfci['N_OUT']   # N passed through unswitched


# ═══════════════════════════════════════════════════════════════════════════
# Generate
# ═══════════════════════════════════════════════════════════════════════════

os.makedirs('output', exist_ok=True)
generate('output/sample')
