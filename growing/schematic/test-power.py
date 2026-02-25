"""
test-power.py — Sheet 1: Power Supply (test render)

Schematic layout:
  AC Mains (L / N / PE)
    → GFCI/RCD 30mA
    → 5A fuse holder (on L line only)
    → split to 5V/3A PSU (HDR-15-5)  →  +5V_RAIL, GND
             + 12V/1A PSU             →  +12V_RAIL, GND

Run:
    python3 test-power.py
Output:
    output/test-power.svg
"""

import schemdraw
import schemdraw.elements as elm

schemdraw.use('svg')


def ic_box(label, left_pins, right_pins, pinspacing=0.85, edgepadH=0.5, edgepadW=0.7):
    """Build an Ic element with auto-spaced left/right pins."""
    # Use slot='N/M' for explicit slot placement, or just let schemdraw auto-space.
    # Auto-spacing: don't specify pos at all — schemdraw evenly distributes pins.
    # To balance left vs right, pad the shorter side with ghost pins.
    n_left  = len(left_pins)
    n_right = len(right_pins)

    pins = []
    for name in left_pins:
        pins.append(elm.IcPin(name=name, side='left'))
    for name in right_pins:
        pins.append(elm.IcPin(name=name, side='right'))

    return elm.Ic(
        pins=pins,
        edgepadW=edgepadW,
        edgepadH=edgepadH,
        pinspacing=pinspacing,
        label=label,
    )


with schemdraw.Drawing(show=False) as d:
    d.config(fontsize=10, unit=3.5, lw=1.5)

    # ── Mains terminal block ─────────────────────────────────────────────────
    # Three input terminals spaced by the pin pitch of the GFCI box.
    # We'll build the GFCI first to get its exact pin positions, then wire up.

    # Build GFCI box
    gfci = ic_box(
        'GFCI / RCD\n30 mA',
        left_pins=['L_IN', 'N_IN', 'PE_IN'],
        right_pins=['L_OUT', 'N_OUT', 'PE_OUT'],
        pinspacing=0.85,
    )
    # Place GFCI at origin, anchor at L_IN (top-left pin)
    d += gfci.at((0, 0)).anchor('L_IN')

    L_in_pos  = gfci.absanchors['L_IN']
    N_in_pos  = gfci.absanchors['N_IN']
    PE_in_pos = gfci.absanchors['PE_IN']

    # Mains input stubs (wires leading into GFCI left pins)
    STUB = 1.5
    d += elm.Dot(open=True).at((L_in_pos[0] - STUB,  L_in_pos[1])).label('AC L',  loc='left')
    d += elm.Line().at((L_in_pos[0]  - STUB, L_in_pos[1])).to(L_in_pos)
    d += elm.Dot(open=True).at((N_in_pos[0]  - STUB, N_in_pos[1])).label('AC N',  loc='left')
    d += elm.Line().at((N_in_pos[0]  - STUB, N_in_pos[1])).to(N_in_pos)
    d += elm.Dot(open=True).at((PE_in_pos[0] - STUB, PE_in_pos[1])).label('PE',   loc='left')
    d += elm.Line().at((PE_in_pos[0] - STUB, PE_in_pos[1])).to(PE_in_pos)

    # GFCI output pin positions
    L_post_gfci  = gfci.absanchors['L_OUT']
    N_post_gfci  = gfci.absanchors['N_OUT']
    PE_post_gfci = gfci.absanchors['PE_OUT']

    # ── 5A fuse on L line ────────────────────────────────────────────────────
    d += (fuse := elm.FuseIEC().at(L_post_gfci).right().label('5 A', loc='top'))
    L_post_fuse = d.here

    # Compute fuse horizontal span
    fuse_span = L_post_fuse[0] - L_post_gfci[0]

    # N and PE pass-through lines (same span)
    d += elm.Line().at(N_post_gfci).right(fuse_span)
    N_post_fuse = d.here
    d += elm.Line().at(PE_post_gfci).right(fuse_span)
    PE_post_fuse = d.here

    # ── Junction dots: split to two PSUs ────────────────────────────────────
    STUB2 = 1.0   # short stub before junction dot
    d += elm.Line().at(L_post_fuse).right(STUB2)
    d += (L_jct := elm.Dot())
    L_jct_pos = d.here

    d += elm.Line().at(N_post_fuse).right(STUB2)
    d += (N_jct := elm.Dot())
    N_jct_pos = d.here

    d += elm.Line().at(PE_post_fuse).right(STUB2)
    d += (PE_jct := elm.Dot())
    PE_jct_pos = d.here

    # ── 5V / 3A PSU (HDR-15-5) ──────────────────────────────────────────────
    PSU_GAP = 2.0   # horizontal gap from junction to PSU left pin

    psu5 = ic_box(
        '5 V / 3 A PSU\n(HDR-15-5)',
        left_pins=['L', 'N', 'PE'],
        right_pins=['+5V', 'GND'],
        pinspacing=0.85,
    )
    d += psu5.at((L_jct_pos[0] + PSU_GAP, L_jct_pos[1])).anchor('L')

    d += elm.Line().at(L_jct_pos).to(psu5.absanchors['L'])
    d += elm.Line().at(N_jct_pos).to(psu5.absanchors['N'])
    d += elm.Line().at(PE_jct_pos).to(psu5.absanchors['PE'])

    # 5V outputs
    d += elm.Line().at(psu5.absanchors['+5V']).right(1.4)
    d += elm.Dot(open=True).label('+5V_RAIL', loc='right')
    d += elm.Line().at(psu5.absanchors['GND']).right(1.4)
    d += elm.Ground()

    # ── 12V / 1A PSU ────────────────────────────────────────────────────────
    # Place below: vertical drop from junctions
    psu12 = ic_box(
        '12 V / 1 A PSU',
        left_pins=['L', 'N', 'PE'],
        right_pins=['+12V', 'GND'],
        pinspacing=0.85,
    )
    # Find how far down to place the 12V PSU so its L pin aligns with the drop wire
    # We want the L pin to be at same x as psu5 L but some y below.
    # Drop distance: enough to clear psu5 height + spacing
    DROP = 4.0   # units drop from L_jct to 12V PSU L pin
    # schemdraw y increases upward; subtract to place below junction
    d += psu12.at((L_jct_pos[0] + PSU_GAP,
                   L_jct_pos[1] - DROP * d.unit)).anchor('L')

    psu12_L_pos  = psu12.absanchors['L']
    psu12_N_pos  = psu12.absanchors['N']
    psu12_PE_pos = psu12.absanchors['PE']

    # Drop wires: schemdraw down(x) decreases y by x; drop = jct.y - psu12.y
    d += elm.Line().at(L_jct_pos).down(L_jct_pos[1] - psu12_L_pos[1])
    d += elm.Dot()   # junction dot at corner
    d += elm.Line().to(psu12_L_pos)

    d += elm.Line().at(N_jct_pos).down(N_jct_pos[1] - psu12_N_pos[1])
    d += elm.Dot()
    d += elm.Line().to(psu12_N_pos)

    d += elm.Line().at(PE_jct_pos).down(PE_jct_pos[1] - psu12_PE_pos[1])
    d += elm.Dot()
    d += elm.Line().to(psu12_PE_pos)

    # 12V outputs
    d += elm.Line().at(psu12.absanchors['+12V']).right(1.4)
    d += elm.Dot(open=True).label('+12V_RAIL', loc='right')
    d += elm.Line().at(psu12.absanchors['GND']).right(1.4)
    d += elm.Ground()

    d.save('output/test-power.svg')

print("Saved: output/test-power.svg")
