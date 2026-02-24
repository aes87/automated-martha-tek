"""
generate.py — Martha Tent Controller full schematic

Generates a complete logical netlist SVG for the DIY ESP32-S3 Martha tent
controller. Four logical sections mirror the planned schematic sheets:

  Sheet 1 — Power:          AC mains → GFCI/RCD → fuse → 5V/3A + 12V/1A PSUs
  Sheet 2 — Controller:     ESP32-S3, failsafe panel, 8-ch relay board
  Sheet 3 — Sensor bus:     TCA9548A I2C mux, SHT45×3, SCD30, AS7341, DS18B20×5
  Sheet 4 — Loads/ADC:      relay contacts, per-load fuses (terminal points),
                             water level sensor + ADC protection circuit

GPIO assignments (ESP32-S3 DevKitC-1, BOARD_S3 in firmware):
  GPIO38        = RELAY_FOGGER    GPIO39/MTCK   = RELAY_TUB_FAN
  GPIO18        = RELAY_EXHAUST   GPIO19        = RELAY_INTAKE
  GPIO40/MTDO   = RELAY_UVC       GPIO41/MTDI   = RELAY_LIGHTS
  GPIO42/MTMS   = RELAY_PUMP      GPIO46        = RELAY_SPARE
  GPIO21        = I2C_SDA         GPIO9         = I2C_SCL
  GPIO4         = ONE_WIRE         GPIO7         = ADC_WATER_LEVEL

Note: firmware uses GPIO47 for spare relay; GPIO46 substituted here as GPIO47
is not exposed in the KiCad ESP32-S3 bare-die symbol (WROOM-1 module pin).

Run:
    python3 generate.py
Output:
    output/martha-controller.svg
"""

import json, os, subprocess, time
from skidl import *
from custom_parts import *


# ─────────────────────────────────────────────────────────────────────────────
# Part templates from bundled KiCad SKIDL libs
# ─────────────────────────────────────────────────────────────────────────────

ESP32S3   = Part('MCU_Espressif',       'ESP32-S3',      tool=SKIDL, dest=TEMPLATE)
TCA9548A  = Part('Interface_Expansion', 'TCA9548APWR',   tool=SKIDL, dest=TEMPLATE)
SHT4x_T   = Part('Sensor_Humidity',     'SHT4x',         tool=SKIDL, dest=TEMPLATE)
AS7341_T  = Part('Sensor_Optical',      'AS7341DLG',     tool=SKIDL, dest=TEMPLATE)
DS18B20_T = Part('Sensor_Temperature',  'DS18B20U',      tool=SKIDL, dest=TEMPLATE)
R_T       = Part('Device',              'R',             tool=SKIDL, dest=TEMPLATE)
C_T       = Part('Device',              'C',             tool=SKIDL, dest=TEMPLATE)
DZ_T      = Part('Device',              'D_Zener',       tool=SKIDL, dest=TEMPLATE)
Fuse_T    = Part('Device',              'Fuse',          tool=SKIDL, dest=TEMPLATE)
PWR_5V    = Part('power', '+5V',   tool=SKIDL, dest=TEMPLATE)
PWR_12V   = Part('power', '+12V',  tool=SKIDL, dest=TEMPLATE)
PWR_GND   = Part('power', 'GND',   tool=SKIDL, dest=TEMPLATE)


def build():
    # ─────────────────────────────────────────────────────────────────────────
    # Nets
    # ─────────────────────────────────────────────────────────────────────────

    # AC mains
    ac_l    = Net('AC_L')
    ac_n    = Net('AC_N')
    ac_pe   = Net('AC_PE')
    l_fused = Net('L_FUSED')      # live rail after GFCI + 5A fuse

    # DC rails
    v5  = Net('+5V')
    v12 = Net('+12V')
    gnd = Net('GND')

    # Relay control (ESP32 GPIO outputs)
    rel = [Net(f'REL_CH{i+1}') for i in range(8)]

    # After failsafe panel (to relay board IN pins via 470Ω drive resistors)
    fs  = [Net(f'FS_CH{i+1}')  for i in range(8)]
    ri  = [Net(f'REL_IN{i+1}') for i in range(8)]

    # I2C
    i2c_sda = Net('I2C_SDA')
    i2c_scl = Net('I2C_SCL')

    # TCA9548A mux channel buses (channels 0–2 used; 3–7 unused)
    ch_sda = [Net(f'CH{i}_SDA') for i in range(3)]
    ch_scl = [Net(f'CH{i}_SCL') for i in range(3)]

    # 1-Wire bus
    one_wire = Net('ONE_WIRE')

    # Water level ADC path
    water_v   = Net('WATER_V')    # voltage across 150Ω shunt (0.6–3.0 V)
    water_adc = Net('WATER_ADC')  # after 1kΩ series R, into ESP32 ADC pin

    # Switched load nets (relay NO contacts → fuse → load)
    ac_loads_net = [Net(f'SW_L_CH{i+1}') for i in range(6)]
    sw_pump_12v  = Net('SW_PUMP_12V')

    # ─────────────────────────────────────────────────────────────────────────
    # Sheet 1 — Power
    # ─────────────────────────────────────────────────────────────────────────

    gfci  = GFCI_RCD()
    f_main = Fuse_T(value='5A', ref='F_MAIN')
    psu5  = PSU_5V3A()
    psu12 = PSU_12V1A()

    # GFCI input
    gfci['L_IN']  += ac_l
    gfci['N_IN']  += ac_n
    gfci['PE_IN'] += ac_pe

    # 5A main fuse on live line after GFCI
    f_main[1] += gfci['L_OUT']
    f_main[2] += l_fused

    # 5V/3A PSU
    psu5['L']         += l_fused
    psu5['N']         += gfci['N_OUT']
    psu5['PE']        += gfci['PE_OUT']
    psu5['+5V_OUT']   += v5
    psu5['GND_OUT']   += gnd

    # 12V/1A PSU
    psu12['L']         += l_fused
    psu12['N']         += gfci['N_OUT']
    psu12['PE']        += gfci['PE_OUT']
    psu12['+12V_OUT']  += v12
    psu12['GND_OUT']   += gnd

    PWR_5V()['~']  += v5
    PWR_12V()['~'] += v12
    PWR_GND()['~'] += gnd

    # ─────────────────────────────────────────────────────────────────────────
    # Sheet 2 — Controller core
    # ─────────────────────────────────────────────────────────────────────────

    mcu   = ESP32S3()
    panel = FailsafePanel()
    relay = Relay_8ch()

    # ESP32-S3 power (DevKitC-1: 5V on USB/5V pin → onboard LDO → 3.3V)
    mcu['VDD3P3']     += v5
    mcu['VDD3P3_CPU'] += v5
    mcu['VDD3P3_RTC'] += v5
    mcu['VDD_SPI']    += v5
    mcu['VDDA']       += v5
    mcu['GND']        += gnd

    # GPIO → relay signals
    # KiCad lib uses IC-package pin names; GPIO→pinname mapping per ESP32-S3 datasheet
    gpio_to_pin = [
        ('GPIO38',        rel[0]),   # Ch1 Fogger      (GPIO38)
        ('MTCK',          rel[1]),   # Ch2 Tub fan     (GPIO39)
        ('GPIO18',        rel[2]),   # Ch3 Exhaust
        ('GPIO19/USB_D-', rel[3]),   # Ch4 Intake      (USB_D- dual-function; not USB here)
        ('MTDO',          rel[4]),   # Ch5 UVC         (GPIO40)
        ('MTDI',          rel[5]),   # Ch6 Lights      (GPIO41)
        ('MTMS',          rel[6]),   # Ch7 Pump        (GPIO42)
        ('GPIO46',        rel[7]),   # Ch8 Spare       (firmware: GPIO47; GPIO46 used here)
    ]
    for pin_name, net in gpio_to_pin:
        mcu[pin_name] += net

    # I2C and analog
    mcu['GPIO21'] += i2c_sda
    mcu['GPIO9']  += i2c_scl
    mcu['GPIO4']  += one_wire
    mcu['GPIO7']  += water_adc

    # I2C pullup resistors (4.7kΩ to 5V)
    r_sda = R_T(value='4.7k', ref='R_SDA'); r_sda[1] += v5; r_sda[2] += i2c_sda
    r_scl = R_T(value='4.7k', ref='R_SCL'); r_scl[1] += v5; r_scl[2] += i2c_scl

    # Failsafe panel — ESP32 inputs
    for i in range(8):
        panel[f'ESP_CH{i+1}'] += rel[i]
    panel['VCC'] += v5
    panel['GND'] += gnd

    # Failsafe panel — relay side outputs
    for i in range(8):
        panel[f'OUT_CH{i+1}'] += fs[i]

    # 470Ω drive resistors: 3.3V logic fix for PC817 optocouplers
    # I = (3.3 − 1.2) / 470Ω ≈ 4.5 mA — within PC817 reliable activation range
    relay_in_pins = ['IN1','IN2','IN3','IN4','IN5','IN6','IN7','IN8']
    for i, rpin in enumerate(relay_in_pins):
        rx = R_T(value='470R', ref=f'R_DR{i+1}')
        rx[1] += fs[i]
        rx[2] += ri[i]
        relay[rpin] += ri[i]

    # Relay board power
    relay['VCC']    += v5
    relay['JD_VCC'] += v5    # coil rail; remove onboard jumper for full isolation
    relay['GND']    += gnd

    # NC contacts unused
    for i in range(1, 9):
        relay[f'NC{i}'] += NC

    # ─────────────────────────────────────────────────────────────────────────
    # Sheet 3 — Sensor bus
    # ─────────────────────────────────────────────────────────────────────────

    mux   = TCA9548A()
    co2   = SCD30_Breakout()
    light = AS7341_T()

    # TCA9548A — address 0x70 (A2/A1/A0 all to GND)
    mux['SDA']      += i2c_sda
    mux['SCL']      += i2c_scl
    mux['VCC']      += v5
    mux['GND']      += gnd
    mux['A0']       += gnd
    mux['A1']       += gnd
    mux['A2']       += gnd
    mux['~{RESET}'] += v5     # active LOW; tie HIGH

    c_mux = C_T(value='100nF', ref='C_MUX'); c_mux[1] += v5; c_mux[2] += gnd

    # Channels 0–2: one SHT45 each (all share I2C addr 0x44; mux selects)
    for i in range(3):
        sht = SHT4x_T()
        mux[f'SD{i}'] += ch_sda[i];  mux[f'SC{i}'] += ch_scl[i]
        sht['SDA']     += ch_sda[i]; sht['SCL']     += ch_scl[i]
        sht['VDD']     += v5;        sht['VSS']     += gnd

    # Channels 3–7 unused
    for i in range(3, 8):
        mux[f'SD{i}'] += NC
        mux[f'SC{i}'] += NC

    # SCD30 on main I2C bus (unique addr 0x61 — no mux needed)
    co2['VIN'] += v5
    co2['GND'] += gnd
    co2['SCL'] += i2c_scl
    co2['SDA'] += i2c_sda
    co2['SEL'] += gnd    # LOW = I2C mode
    co2['3Vo'] += NC

    # AS7341 on main I2C bus (unique addr 0x39)
    light['VDD']  += v5
    light['GND']  += gnd
    light['PGND'] += gnd
    light['SCL']  += i2c_scl
    light['SDA']  += i2c_sda
    light['LDR']  += NC
    light['INT']  += NC
    light['GPIO'] += NC

    # DS18B20 1-Wire probes × 5 (one per shelf; all share one bus)
    r_ow = R_T(value='4.7k', ref='R_OW'); r_ow[1] += v5; r_ow[2] += one_wire

    for i in range(1, 6):
        p = DS18B20_T()
        p['DQ']     += one_wire
        p['V_{DD}'] += v5
        p['GND']    += gnd
        # NC guard-ring pins left floating (ERC will warn; safe to ignore for TO-92)

    # ─────────────────────────────────────────────────────────────────────────
    # Sheet 4 — Loads and ADC protection
    # ─────────────────────────────────────────────────────────────────────────

    # AC relay contacts — live rail → COM; NO → blade fuse → named output net
    # Named output nets become module ports → rendered as outputExt arrows
    ac_load_names  = ['FOGGER', 'TUB_FAN', 'EXHAUST', 'INTAKE', 'UVC', 'LIGHTS']
    ac_fuse_values = ['2A',     '2A',      '1A',      '1A',     '1A',  '3A'    ]

    for i, (name, fval) in enumerate(zip(ac_load_names, ac_fuse_values)):
        relay[f'COM{i+1}'] += l_fused
        fuse = Fuse_T(value=fval, ref=f'F_{name}')
        relay[f'NO{i+1}']  += ac_loads_net[i]
        fuse[1]             += ac_loads_net[i]
        fuse[2]             += Net(f'{name}_HOT')   # switched live output

    # 12VDC pump — Ch7
    relay['COM7'] += v12
    relay['NO7']  += sw_pump_12v
    f_pump = Fuse_T(value='1A', ref='F_PUMP')
    f_pump[1] += sw_pump_12v
    f_pump[2] += Net('PUMP_HOT')                    # 12V pump output

    # Spare relay Ch8
    relay['COM8'] += l_fused
    relay['NO8']  += Net('SPARE_HOT')               # spare output (unfused)

    # ── Water level sensor ADC circuit ──────────────────────────────────────
    # 4–20 mA loop: +12V → sensor → WATER_V node → 150Ω shunt → GND
    # WATER_V = 0.6–3.0 V. Series 1kΩ + 3.3V Zener clamp protect ESP32 ADC.
    #
    #   +12V ──── Sensor(+)
    #              Sensor(IOUT) ── WATER_V ──┬── R_SHUNT(150Ω) ── GND
    #                                         │
    #                                        R_ADC(1kΩ) ── WATER_ADC ── GPIO7
    #                                                          │
    #                                                   D_ADC(3.3V Zener) ── GND

    wls = Water_Level_Sensor()
    wls['VIN']  += v12
    wls['IOUT'] += water_v

    r_shunt = R_T(value='150R', ref='R_SHUNT'); r_shunt[1] += water_v; r_shunt[2] += gnd

    r_adc = R_T(value='1k', ref='R_ADC'); r_adc[1] += water_v; r_adc[2] += water_adc

    dz = DZ_T(value='3.3V BZX84')
    dz['K'] += water_adc
    dz['A'] += gnd

    PWR_5V()['~']  += v5
    PWR_12V()['~'] += v12
    PWR_GND()['~'] += gnd


def _rename_connections(conns, nm):
    """Rename connection keys by SKiDL pin name.

    When multiple numeric pin IDs would map to the same pin name (e.g. both
    pins of R/C/Fuse are named '~'), keep the original numeric keys so no
    connection is silently dropped.
    """
    new_keys = [nm.get(k, k) for k in conns]
    if len(new_keys) != len(set(new_keys)):
        return conns          # collision: keep numeric keys as-is
    return {nm.get(k, k): v for k, v in conns.items()}


# Symbol definitions injected into the netlistsvg skin after generate_svg() writes it.
# s:pid values must match the connection keys produced by _rename_connections above:
#   - R / C / Fuse  →  numeric '1' / '2'  (collision-avoidance kept them numeric)
#   - GND / +5V / +12V  →  '~'  (single pin, no collision)
#   - D_Zener  →  'K' / 'A'  (unique names, renamed correctly)
_SKIN_SYMBOLS = """\

<!-- ── power symbols ──────────────────────────────────────── -->
<g s:type="GND" s:width="20" s:height="20" transform="translate(0,0)">
  <s:alias val="GND"/>
  <text x="22" y="12" class="$cell_id net_name_text" s:attribute="value">GND</text>
  <path d="M10,0 V5 M0,5 H20 M3,10 H17 M6,15 H14" class="$cell_id symbol"/>
  <g s:x="10" s:y="0" s:pid="~" s:position="top"/>
</g>

<g s:type="+5V" s:width="20" s:height="25" transform="translate(0,0)">
  <s:alias val="+5V"/>
  <text x="10" y="1" text-anchor="middle" class="$cell_id net_name_text" s:attribute="value">+5V</text>
  <path d="M10,25 V8 M0,8 H20" class="$cell_id symbol"/>
  <g s:x="10" s:y="25" s:pid="~" s:position="bottom"/>
</g>

<g s:type="+12V" s:width="24" s:height="25" transform="translate(0,0)">
  <s:alias val="+12V"/>
  <text x="12" y="1" text-anchor="middle" class="$cell_id net_name_text" s:attribute="value">+12V</text>
  <path d="M12,25 V8 M0,8 H24" class="$cell_id symbol"/>
  <g s:x="12" s:y="25" s:pid="~" s:position="bottom"/>
</g>

<!-- ── passives ────────────────────────────────────────────── -->
<g s:type="R" s:width="16" s:height="50" transform="translate(0,0)">
  <s:alias val="R"/>
  <text x="18" y="20" class="$cell_id part_ref_text" s:attribute="ref">R</text>
  <text x="18" y="33" class="$cell_id" s:attribute="value">Xk</text>
  <rect x="3" y="10" width="10" height="30" class="$cell_id symbol"/>
  <path d="M8,0 V10 M8,40 V50" class="$cell_id"/>
  <g s:x="8" s:y="0" s:pid="1" s:position="top"/>
  <g s:x="8" s:y="50" s:pid="2" s:position="bottom"/>
</g>

<g s:type="C" s:width="30" s:height="50" transform="translate(0,0)">
  <s:alias val="C"/>
  <text x="32" y="20" class="$cell_id part_ref_text" s:attribute="ref">C</text>
  <text x="32" y="33" class="$cell_id" s:attribute="value">Xu</text>
  <path d="M0,20 H30 M0,30 H30" class="$cell_id symbol"/>
  <path d="M15,0 V20 M15,30 V50" class="$cell_id"/>
  <g s:x="15" s:y="0" s:pid="1" s:position="top"/>
  <g s:x="15" s:y="50" s:pid="2" s:position="bottom"/>
</g>

<g s:type="Fuse" s:width="16" s:height="50" transform="translate(0,0)">
  <s:alias val="Fuse"/>
  <text x="18" y="20" class="$cell_id part_ref_text" s:attribute="ref">F</text>
  <text x="18" y="33" class="$cell_id" s:attribute="value">Xv</text>
  <rect x="3" y="12" width="10" height="26" class="$cell_id symbol"/>
  <path d="M8,0 V12 M8,38 V50" class="$cell_id"/>
  <g s:x="8" s:y="0" s:pid="1" s:position="top"/>
  <g s:x="8" s:y="50" s:pid="2" s:position="bottom"/>
</g>

<g s:type="D_Zener" s:width="20" s:height="50" transform="translate(0,0)">
  <s:alias val="D_Zener"/>
  <text x="22" y="20" class="$cell_id part_ref_text" s:attribute="ref">D</text>
  <text x="22" y="33" class="$cell_id" s:attribute="value">Xv</text>
  <path d="M0,15 H20 L10,35 Z" class="$cell_id symbol"/>
  <path d="M0,15 V10 M20,15 V20" class="$cell_id"/>
  <path d="M10,0 V15 M10,35 V50" class="$cell_id"/>
  <g s:x="10" s:y="0" s:pid="K" s:position="top"/>
  <g s:x="10" s:y="50" s:pid="A" s:position="bottom"/>
</g>
"""


# Nets to expose as module-level input/output ports in the final JSON.
# Each entry: cell_ref → [(pin_key_in_clean_json, port_name, direction), ...]
# netlistsvg renders module ports as inputExt/outputExt arrow symbols.
_BOUNDARY_PORTS = {
    # AC mains entry — GFCI input pins are the circuit boundary
    'U1': [
        ('L_IN',  'AC_L',  'input'),
        ('N_IN',  'AC_N',  'input'),
        ('PE_IN', 'AC_PE', 'input'),
    ],
    # Fuse outputs — switched live to each AC load
    'F_FOGGER':  [('2', 'FOGGER_HOT',  'output')],
    'F_TUB_FAN': [('2', 'TUB_FAN_HOT', 'output')],
    'F_EXHAUST': [('2', 'EXHAUST_HOT', 'output')],
    'F_INTAKE':  [('2', 'INTAKE_HOT',  'output')],
    'F_UVC':     [('2', 'UVC_HOT',     'output')],
    'F_LIGHTS':  [('2', 'LIGHTS_HOT',  'output')],
    'F_PUMP':    [('2', 'PUMP_HOT',    'output')],
    # Spare relay NO contact (unfused)
    'U5':        [('NO8', 'SPARE_HOT', 'output')],
}


def _add_module_ports(data):
    """Promote boundary nets to module-level ports so they render as arrows."""
    for mod in data['modules'].values():
        ports = mod.setdefault('ports', {})
        cells = mod.get('cells', {})
        for ref, pin_defs in _BOUNDARY_PORTS.items():
            cell = cells.get(ref)
            if not cell:
                continue
            for pin_key, port_name, direction in pin_defs:
                bits = cell['connections'].get(pin_key, [])
                if bits:
                    ports[port_name] = {'direction': direction, 'bits': bits}


def _augment_skin(skin_path):
    """Inject custom component symbols into the netlistsvg skin SVG."""
    with open(skin_path) as f:
        svg = f.read()
    svg = svg.replace('</svg>', _SKIN_SYMBOLS + '</svg>', 1)
    with open(skin_path, 'w') as f:
        f.write(svg)


def render(outfile):
    """Post-process SKiDL JSON and re-render with netlistsvg."""
    pin_map = {p.ref: {str(pin.num): pin.name for pin in p}
               for p in default_circuit.parts}

    generate_svg(file_=outfile)
    time.sleep(0.5)

    json_path  = outfile + '.json'
    skin_path  = outfile + '_skin.svg'
    svg_path   = outfile + '.svg'

    # Augment AFTER generate_svg() overwrites the skin file
    _augment_skin(skin_path)

    with open(json_path) as f:
        data = json.load(f)

    for mod in data['modules'].values():
        for ref, cell in mod['cells'].items():
            nm = pin_map.get(ref, {})
            cell['connections']     = _rename_connections(cell['connections'], nm)
            cell['port_directions'] = _rename_connections(cell['port_directions'], nm)
            cell['attributes']['ref'] = ref
            t = cell['type']
            if t.endswith('_1_'):
                t = t[:-3]
            cell['type'] = t

    _add_module_ports(data)

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


if __name__ == '__main__':
    reset()
    build()

    print('\n=== ERC ===')
    ERC()

    os.makedirs('output', exist_ok=True)
    render('output/martha-controller')
