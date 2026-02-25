"""
colorize.py — Post-process a netlistsvg SVG output to color wires by net function.

Reads the *_clean.json netlist to identify each net by its connected pins,
assigns a color by electrical category, and injects a <style> block into the SVG.

Color design principles:
  - AC mains use IEC standard colors (red/blue/green) — universally understood
  - 8 relay channels use the Okabe-Ito colorblind-safe palette
  - Colors are reused end-to-end along a signal chain (GPIO → relay contact → load)
    to make signal flow traceable. Reuse across non-adjacent areas is intentional.
  - Maximum hue separation between wires that run adjacent at the same component

Usage (standalone):
    python3 colorize.py output/martha-controller

Or import:
    from colorize import colorize
    colorize('output/martha-controller')
"""

import json, sys


# ─────────────────────────────────────────────────────────────────────────────
# Color map: net_bit → hex color string
# Built by inspecting which ref.pin pairs are connected to each net.
# ─────────────────────────────────────────────────────────────────────────────

# Rule format: (color_hex, [(ref, pin_substring), ...])
# A net matches if ANY listed (ref, pin) pair appears in its connection list.
# ref is matched exactly; pin_substring is matched as a substring of the pin name.
# First matching rule wins.
#
# ADJACENT WIRE GROUPS (need maximum contrast within each group):
#
#   At ESP32 (U4):
#     GPIO38, MTCK, GPIO18, GPIO19, MTDO, MTDI, MTMS, GPIO46  ← 8 relay signals
#     GPIO21 (SDA), GPIO9 (SCL), GPIO4 (1-Wire), GPIO7 (ADC)  ← 4 more
#     → 12 adjacent wires: relay channels get Okabe-Ito colors (8 distinct)
#       I2C/1-Wire/ADC get high-contrast colors from remaining hue space
#
#   At TCA9548A (U6):
#     SDA, SCL on one side; SD0/SC0, SD1/SC1, SD2/SC2 on other
#     → Main I2C (2 wires) + 6 channel wires: use primary-color trio for channels
#
#   At relay board (U5):
#     IN1-8 (relay signal continuations), NO1-8 (switched loads)
#     → relay channels reuse same colors end-to-end for traceability

NET_RULES = [

    # ── AC mains — IEC standard wire colors ──────────────────────────────────
    # All live conductors: bold red (#CC0000)
    # Covers: raw AC_L, post-GFCI L_OUT, fused L_FUSED rail, relay COMs for AC loads
    ("#CC0000", [
        ("U1",     "L_IN"),
        ("U1",     "L_OUT"),
        ("F_MAIN", None),      # main fuse live side
        ("U2",     "L"),       # PSU 5V live
        ("U3",     "L"),       # PSU 12V live
        ("U5",     "COM1"),    ("U5", "COM2"), ("U5", "COM3"),
        ("U5",     "COM4"),    ("U5", "COM5"), ("U5", "COM6"),
        ("U5",     "COM8"),    # spare relay COM
    ]),

    # All neutral conductors: bold blue (#0055DD)
    ("#0055DD", [
        ("U1",       "N_IN"),
        ("U1",       "N_OUT"),
    ]),

    # All protective earth: bold green (#008800)
    ("#008800", [
        ("U1",    "PE_IN"),
        ("U1",    "PE_OUT"),
        ("U2",    "PE"),
        ("U3",    "PE"),
    ]),

    # ── DC power rails ────────────────────────────────────────────────────────
    # +5V: saturated orange — clearly different from AC red
    ("#FF7700", [
        ("#PWR1", "~"),
        ("#PWR4", "~"),
        ("U2",    "+5V_OUT"),
    ]),

    # +12V: dark gold — clearly different from orange
    ("#AA8800", [
        ("#PWR2", "~"),
        ("#PWR5", "~"),
        ("U3",    "+12V_OUT"),
        ("U5",    "COM7"),   # 12V pump relay COM
    ]),

    # GND: dark charcoal (not pure black — avoids merging with schematic outlines)
    ("#444444", [
        ("#PWR3", "~"),
        ("#PWR6", "~"),
        ("C_MUX", None),
        ("D1",    "A"),
    ]),

    # ── I2C main bus (enters TCA on one side, ESP32 on other) ─────────────────
    # SDA: hot magenta — unambiguous, wide hue separation from all relay channels
    ("#DD0088", [
        ("R_SDA", None),
        ("U4",    "GPIO21"),
        ("U6",    "SDA"),
        ("U7",    "SDA"),   # SCD30
        ("U8",    "SDA"),   # AS7341
    ]),

    # SCL: electric indigo — clearly different from magenta and from relay channels
    ("#5500CC", [
        ("R_SCL", None),
        ("U4",    "GPIO9"),
        ("U6",    "SCL"),
        ("U7",    "SCL"),
        ("U8",    "SCL"),
    ]),

    # ── TCA9548A mux channels (primary-color trio for max distinction) ─────────
    # Channel 0 → SHT45 shelf 1: red-orange (reuse OK — far from AC live red)
    ("#FF3300", [
        ("U6", "SD0"), ("U9", "SDA"),
        ("U6", "SC0"), ("U9", "SCL"),
    ]),

    # Channel 1 → SHT45 shelf 2: amber-yellow (distinct from orange, warm)
    ("#CCAA00", [
        ("U6",  "SD1"), ("U10", "SDA"),
        ("U6",  "SC1"), ("U10", "SCL"),
    ]),

    # Channel 2 → SHT45 shelf 3: medium blue (the third primary, clearly distinct)
    ("#2266EE", [
        ("U6",  "SD2"), ("U11", "SDA"),
        ("U6",  "SC2"), ("U11", "SCL"),
    ]),

    # ── 1-Wire bus ────────────────────────────────────────────────────────────
    # Spring green — wide separation from PE green (#008800) in hue and brightness
    ("#00CC66", [
        ("R_OW", None),
        ("U4",   "GPIO4"),
        ("U12",  "DQ"), ("U13", "DQ"), ("U14", "DQ"),
        ("U15",  "DQ"), ("U16", "DQ"),
    ]),

    # ── Water level ADC path ──────────────────────────────────────────────────
    # Fuchsia: distinct from magenta (SDA) and pink relay channel (Ch4)
    ("#FF44BB", [
        ("U17",    "IOUT"),
        ("R_SHUNT", None),
        ("R_ADC",   None),
        ("U4",     "GPIO7"),
        ("D1",     "K"),
    ]),

    # ── Relay channels — Okabe-Ito colorblind-safe palette ────────────────────
    # Colors applied end-to-end: ESP32 GPIO → failsafe → drive R → relay IN
    #                            → relay NO → load fuse → load connector
    # This makes each load's signal path fully traceable by color.

    # Ch1 Fogger — warm amber: #E69F00 (Okabe-Ito orange)
    ("#E69F00", [
        ("U4",      "GPIO38"),  ("SW1",     "ESP_CH1"),
        ("SW1",     "OUT_CH1"), ("R_DR1",   None),
        ("U5",      "IN1"),     ("U5",      "NO1"),
        ("F_FOGGER", None),
    ]),

    # Ch2 Tub fan — sky blue: #56B4E9 (Okabe-Ito sky blue)
    ("#56B4E9", [
        ("U4",       "MTCK"),   ("SW1",      "ESP_CH2"),
        ("SW1",      "OUT_CH2"),("R_DR2",    None),
        ("U5",       "IN2"),    ("U5",       "NO2"),
        ("F_TUB_FAN", None),
    ]),

    # Ch3 Exhaust fan — bluish green: #009E73 (Okabe-Ito bluish green)
    ("#009E73", [
        ("U4",       "GPIO18"),  ("SW1",     "ESP_CH3"),
        ("SW1",      "OUT_CH3"), ("R_DR3",   None),
        ("U5",       "IN3"),     ("U5",      "NO3"),
        ("F_EXHAUST", None),
    ]),

    # Ch4 Intake fan — reddish rose: #CC79A7 (Okabe-Ito reddish purple)
    ("#CC79A7", [
        ("U4",      "GPIO19"),   ("SW1",     "ESP_CH4"),
        ("SW1",     "OUT_CH4"),  ("R_DR4",   None),
        ("U5",      "IN4"),      ("U5",      "NO4"),
        ("F_INTAKE", None),
    ]),

    # Ch5 UVC — vermillion: #D55E00 (Okabe-Ito vermillion)
    ("#D55E00", [
        ("U4",    "MTDO"),    ("SW1",   "ESP_CH5"),
        ("SW1",   "OUT_CH5"), ("R_DR5", None),
        ("U5",    "IN5"),     ("U5",    "NO5"),
        ("F_UVC", None),
    ]),

    # Ch6 Lights — deep blue: #0072B2 (Okabe-Ito blue)
    ("#0072B2", [
        ("U4",      "MTDI"),    ("SW1",     "ESP_CH6"),
        ("SW1",     "OUT_CH6"), ("R_DR6",   None),
        ("U5",      "IN6"),     ("U5",      "NO6"),
        ("F_LIGHTS", None),
    ]),

    # Ch7 Pump — amethyst purple (not in Okabe-Ito but fills the purple gap)
    ("#9922AA", [
        ("U4",    "MTMS"),    ("SW1",    "ESP_CH7"),
        ("SW1",   "OUT_CH7"), ("R_DR7",  None),
        ("U5",    "IN7"),     ("U5",     "NO7"),
        ("F_PUMP", None),
    ]),

    # Ch8 Spare — teal: fills the cyan gap between sky blue and green
    ("#009999", [
        ("U4",    "GPIO46"),  ("SW1",     "ESP_CH8"),
        ("SW1",   "OUT_CH8"), ("R_DR8",   None),
        ("U5",    "IN8"),     ("U5",      "NO8"),
    ]),
]


def build_net_color_map(clean_json_path):
    """Return {net_bit: hex_color} for every matched net."""
    with open(clean_json_path) as f:
        data = json.load(f)

    # Invert connections: (ref, pin) → set of bits
    pin_to_bits = {}
    for mod in data["modules"].values():
        for ref, cell in mod["cells"].items():
            for pin, bits in cell["connections"].items():
                for bit in bits:
                    if isinstance(bit, int):
                        pin_to_bits.setdefault((ref, pin), set()).add(bit)

    color_map = {}
    for color, identifiers in NET_RULES:
        matched_bits = set()
        for ref_key, pin_sub in identifiers:
            for (ref, pin), bits in pin_to_bits.items():
                if ref == ref_key and (pin_sub is None or pin_sub in pin):
                    matched_bits |= bits
        for bit in matched_bits:
            if bit not in color_map:
                color_map[bit] = color

    return color_map


def colorize(base_path):
    """Read base_path_clean.json + base_path.svg, write base_path_color.svg."""
    json_path = base_path + "_clean.json"
    svg_path  = base_path + ".svg"
    out_path  = base_path + "_color.svg"

    color_map = build_net_color_map(json_path)

    # Group net IDs by color for compact CSS
    color_to_nets = {}
    for bit, color in color_map.items():
        color_to_nets.setdefault(color, []).append(f"net_{bit}")

    css_lines = []
    for color, net_classes in sorted(color_to_nets.items()):
        selectors = ", ".join(f"line.{nc}, path.{nc}" for nc in sorted(net_classes))
        css_lines.append(f"  {selectors} {{ stroke: {color}; stroke-width: 2px; }}")

    injected = "\n".join(css_lines)

    with open(svg_path) as f:
        svg = f.read()

    svg = svg.replace(
        "</style>",
        f"</style>\n  <style id='net-colors'>\n{injected}\n  </style>",
        1
    )

    with open(out_path, "w") as f:
        f.write(svg)

    print(f"Saved: {out_path}  ({len(color_map)} nets colored)")
    return out_path


if __name__ == "__main__":
    base = sys.argv[1] if len(sys.argv) > 1 else "output/martha-controller"
    colorize(base)
