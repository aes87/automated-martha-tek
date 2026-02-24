"""
custom_parts.py — SKiDL part definitions for components not in the bundled KiCad libs.

These are end-product modules (breakout boards, relay boards, PSUs) rather than
discrete ICs, so they have no equivalent in the standard KiCad symbol library.

Usage:
    from custom_parts import *

All parts are exported as TEMPLATE Parts ready to instantiate with part().
"""

from skidl import *
from skidl.pin import pin_types

# ─── AC power entry ───────────────────────────────────────────────────────────

GFCI_RCD = Part(
    name='GFCI_RCD',
    ref_prefix='U',
    tool=SKIDL,
    dest=TEMPLATE,
    description='GFCI/RCD 30mA residual current device',
    pins=[
        Pin(num='1', name='L_IN',   func=pin_types.PWRIN),
        Pin(num='2', name='N_IN',   func=pin_types.PWRIN),
        Pin(num='3', name='PE_IN',  func=pin_types.PWRIN),
        Pin(num='4', name='L_OUT',  func=pin_types.PWROUT),
        Pin(num='5', name='N_OUT',  func=pin_types.PWROUT),
        Pin(num='6', name='PE_OUT', func=pin_types.PWROUT),
    ],
)

# ─── DIN-rail PSUs ────────────────────────────────────────────────────────────

PSU_5V3A = Part(
    name='PSU_5V3A',
    ref_prefix='U',
    tool=SKIDL,
    dest=TEMPLATE,
    description='5V 3A DIN-rail PSU (Meanwell HDR-30-5 or Mornsun equiv)',
    keywords='PSU 5V 3A Meanwell HDR-30-5',
    pins=[
        Pin(num='1', name='L',       func=pin_types.PWRIN),
        Pin(num='2', name='N',       func=pin_types.PWRIN),
        Pin(num='3', name='PE',      func=pin_types.PWRIN),
        Pin(num='4', name='+5V_OUT', func=pin_types.PWROUT),
        Pin(num='5', name='GND_OUT', func=pin_types.PASSIVE),  # common GND reference
    ],
)

PSU_12V1A = Part(
    name='PSU_12V1A',
    ref_prefix='U',
    tool=SKIDL,
    dest=TEMPLATE,
    description='12V 1A DIN-rail PSU (Meanwell HDR-15-12 or equiv)',
    keywords='PSU 12V 1A Meanwell HDR-15-12',
    pins=[
        Pin(num='1', name='L',        func=pin_types.PWRIN),
        Pin(num='2', name='N',        func=pin_types.PWRIN),
        Pin(num='3', name='PE',       func=pin_types.PWRIN),
        Pin(num='4', name='+12V_OUT', func=pin_types.PWROUT),
        Pin(num='5', name='GND_OUT',  func=pin_types.PASSIVE),  # common GND reference
    ],
)

# ─── 8-channel relay board ────────────────────────────────────────────────────
# PC817 optocoupler inputs. Active LOW. Standard board has 1kΩ IN resistors —
# replace with 470Ω for reliable 3.3V drive (see hardware reference).
# JD_VCC is the relay coil rail. Remove onboard jumper and connect separately
# from VCC (optocoupler logic rail) to achieve full optical isolation.

Relay_8ch = Part(
    name='Relay_8ch',
    ref_prefix='U',
    tool=SKIDL,
    dest=TEMPLATE,
    description='8-channel 5V opto-isolated relay module (PC817, active LOW)',
    keywords='relay 8ch opto isolated 5V',
    pins=[
        # Logic supply (3.3–5V for optocoupler side)
        Pin(num='1',  name='VCC',    func=pin_types.PWRIN),
        Pin(num='2',  name='GND',    func=pin_types.PWRIN),
        # Relay coil supply (5V, isolated from logic via removed jumper)
        Pin(num='3',  name='JD_VCC', func=pin_types.PWRIN),
        # Signal inputs (active LOW)
        Pin(num='4',  name='IN1',    func=pin_types.INPUT),
        Pin(num='5',  name='IN2',    func=pin_types.INPUT),
        Pin(num='6',  name='IN3',    func=pin_types.INPUT),
        Pin(num='7',  name='IN4',    func=pin_types.INPUT),
        Pin(num='8',  name='IN5',    func=pin_types.INPUT),
        Pin(num='9',  name='IN6',    func=pin_types.INPUT),
        Pin(num='10', name='IN7',    func=pin_types.INPUT),
        Pin(num='11', name='IN8',    func=pin_types.INPUT),
        # Relay contacts — COM/NO/NC per channel
        Pin(num='12', name='COM1',   func=pin_types.PASSIVE),
        Pin(num='13', name='NO1',    func=pin_types.PASSIVE),
        Pin(num='14', name='NC1',    func=pin_types.PASSIVE),
        Pin(num='15', name='COM2',   func=pin_types.PASSIVE),
        Pin(num='16', name='NO2',    func=pin_types.PASSIVE),
        Pin(num='17', name='NC2',    func=pin_types.PASSIVE),
        Pin(num='18', name='COM3',   func=pin_types.PASSIVE),
        Pin(num='19', name='NO3',    func=pin_types.PASSIVE),
        Pin(num='20', name='NC3',    func=pin_types.PASSIVE),
        Pin(num='21', name='COM4',   func=pin_types.PASSIVE),
        Pin(num='22', name='NO4',    func=pin_types.PASSIVE),
        Pin(num='23', name='NC4',    func=pin_types.PASSIVE),
        Pin(num='24', name='COM5',   func=pin_types.PASSIVE),
        Pin(num='25', name='NO5',    func=pin_types.PASSIVE),
        Pin(num='26', name='NC5',    func=pin_types.PASSIVE),
        Pin(num='27', name='COM6',   func=pin_types.PASSIVE),
        Pin(num='28', name='NO6',    func=pin_types.PASSIVE),
        Pin(num='29', name='NC6',    func=pin_types.PASSIVE),
        Pin(num='30', name='COM7',   func=pin_types.PASSIVE),
        Pin(num='31', name='NO7',    func=pin_types.PASSIVE),
        Pin(num='32', name='NC7',    func=pin_types.PASSIVE),
        Pin(num='33', name='COM8',   func=pin_types.PASSIVE),
        Pin(num='34', name='NO8',    func=pin_types.PASSIVE),
        Pin(num='35', name='NC8',    func=pin_types.PASSIVE),
    ],
)

# ─── Failsafe panel ───────────────────────────────────────────────────────────
# DPDT master switch (AUTO/MANUAL) + 4 SPST group switches.
# In AUTO: ESP32 GPIO → relay IN pins directly.
# In MANUAL: group switches → relay IN pins; ESP32 disconnected.
# Represented as a single block; internal switch topology in hardware reference.

FailsafePanel = Part(
    name='FailsafePanel',
    ref_prefix='SW',
    tool=SKIDL,
    dest=TEMPLATE,
    description='Manual failsafe panel: DPDT AUTO/MANUAL master + 4x SPST group switches',
    keywords='failsafe manual override switch panel',
    pins=[
        # ESP32 GPIO inputs (AUTO mode)
        Pin(num='1',  name='ESP_CH1',  func=pin_types.INPUT),
        Pin(num='2',  name='ESP_CH2',  func=pin_types.INPUT),
        Pin(num='3',  name='ESP_CH3',  func=pin_types.INPUT),
        Pin(num='4',  name='ESP_CH4',  func=pin_types.INPUT),
        Pin(num='5',  name='ESP_CH5',  func=pin_types.INPUT),
        Pin(num='6',  name='ESP_CH6',  func=pin_types.INPUT),
        Pin(num='7',  name='ESP_CH7',  func=pin_types.INPUT),
        Pin(num='8',  name='ESP_CH8',  func=pin_types.INPUT),
        # Outputs to relay board IN pins
        Pin(num='9',  name='OUT_CH1',  func=pin_types.OUTPUT),
        Pin(num='10', name='OUT_CH2',  func=pin_types.OUTPUT),
        Pin(num='11', name='OUT_CH3',  func=pin_types.OUTPUT),
        Pin(num='12', name='OUT_CH4',  func=pin_types.OUTPUT),
        Pin(num='13', name='OUT_CH5',  func=pin_types.OUTPUT),
        Pin(num='14', name='OUT_CH6',  func=pin_types.OUTPUT),
        Pin(num='15', name='OUT_CH7',  func=pin_types.OUTPUT),
        Pin(num='16', name='OUT_CH8',  func=pin_types.OUTPUT),
        # Pull-high rail for manual switches (5V logic via relay VCC)
        Pin(num='17', name='VCC',      func=pin_types.PWRIN),
        Pin(num='18', name='GND',      func=pin_types.PWRIN),
    ],
)

# ─── Sensors — CO2 ────────────────────────────────────────────────────────────

# Adafruit SCD-30 breakout #4867 — Sensirion NDIR dual-channel CO2/RH/T sensor
# I2C address: 0x61
# SEL pin: LOW = I2C mode; HIGH = Modbus. Wire to GND for I2C.
# https://learn.adafruit.com/adafruit-scd30/pinouts
SCD30_Breakout = Part(
    name='SCD30_Breakout',
    ref_prefix='U',
    tool=SKIDL,
    dest=TEMPLATE,
    description='Adafruit SCD-30 CO2/RH/T breakout (I2C 0x61, NDIR)',
    keywords='SCD30 CO2 humidity temperature I2C Adafruit NDIR',
    pins=[
        Pin(num='1', name='VIN', func=pin_types.PWRIN),
        Pin(num='2', name='3Vo', func=pin_types.PWROUT),
        Pin(num='3', name='GND', func=pin_types.PWRIN),
        Pin(num='4', name='SCL', func=pin_types.INPUT),
        Pin(num='5', name='SDA', func=pin_types.BIDIR),
        Pin(num='6', name='SEL', func=pin_types.INPUT),   # LOW=I2C, HIGH=Modbus
    ],
)

# Adafruit SCD-40 breakout — photoacoustic CO2/RH/T (not selected for this build;
# 400–2000 ppm ceiling disqualifies for oyster/shiitake. Defined for reference.
# https://learn.adafruit.com/adafruit-scd-40-and-scd-41/pinouts
SCD40_Breakout = Part(
    name='SCD40_Breakout',
    ref_prefix='U',
    tool=SKIDL,
    dest=TEMPLATE,
    description='Adafruit SCD-40 CO2/RH/T breakout (I2C 0x62, photoacoustic)',
    keywords='SCD40 CO2 humidity temperature I2C Adafruit photoacoustic',
    pins=[
        Pin(num='1', name='VIN', func=pin_types.PWRIN),
        Pin(num='2', name='3Vo', func=pin_types.PWROUT),
        Pin(num='3', name='GND', func=pin_types.PWRIN),
        Pin(num='4', name='SCL', func=pin_types.INPUT),
        Pin(num='5', name='SDA', func=pin_types.BIDIR),
    ],
)

# ─── Water level sensor ───────────────────────────────────────────────────────
# Submersible hydrostatic pressure sensor, 4–20 mA current loop output.
# Requires 12–24VDC supply. Wire 150Ω shunt resistor across signal lines
# to convert 4–20 mA → 0.6–3.0 V for ESP32 ADC (GPIO 7 on S3).
# See hardware reference for shunt + Zener ADC protection circuit.

Water_Level_Sensor = Part(
    name='Water_Level_Sensor',
    ref_prefix='U',
    tool=SKIDL,
    dest=TEMPLATE,
    description='Submersible hydrostatic 4-20mA pressure sensor (0–0.5m, 12V)',
    keywords='water level pressure sensor 4-20mA submersible',
    pins=[
        Pin(num='1', name='VIN',  func=pin_types.PWRIN),    # 12–24VDC supply
        Pin(num='2', name='IOUT', func=pin_types.OUTPUT),   # 4–20 mA current output
    ],
)
