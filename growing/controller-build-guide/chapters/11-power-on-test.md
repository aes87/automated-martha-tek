# Chapter 11 — Initial Power-On Test

**What you'll do:** Five structured tests that verify the enclosure is safe and
functional before any load cables are connected.

**Prerequisites:** Chapter 10 complete (enclosure sealed, all checklists passed).
Computer with serial terminal (115200 baud), USB-C cable.

> [!CAUTION]
> All load cables must be unplugged from the relay NO terminals before this chapter.
> The enclosure will be powered but no mains-connected loads will be active.
> This is the most important single safety step before first power-on.

---

## Before Power-On: Remove All Load Cables

1. Open the lid (enclosure still unplugged from wall).
2. At the relay board load terminals: confirm all NO terminals are empty. Only COM
   terminals should have wires (from the fuse block).
3. Close the lid.

**✓ Check:** All relay NO terminals confirmed empty.

---

## Setup: Serial Monitor

1. Connect the ESP32-S3 USB-C to your computer.
2. Open Arduino IDE Serial Monitor or PlatformIO terminal at **115200 baud**.
3. Nothing should appear yet — the board is not yet powered from the enclosure supply.

<details>
<summary><strong>[?] 115200 baud:</strong></summary>

The communication rate between the ESP32 and your serial
monitor. If set to a different rate (e.g. 9600), you will see garbled characters
instead of readable text.

</details>

---

## Test 1 — Boot-State Check

**What this tests:** That no relay fires during the ESP32 startup sequence.

1. Plug the enclosure into the wall.
2. In the first 5–10 seconds, listen and watch:
   - **No relay should click.**
   - No relay LED should illuminate.
   - The GFCI/RCD should not trip.
3. After ~5 seconds, the firmware boot lock expires. Relay activity after this
   point is expected.

**What you should see:** Silence for the first 5 seconds. Serial monitor begins
showing boot log messages.

**If a relay clicks at power-on (before firmware runs):**
1. Unplug immediately.
2. Verify all 10 kΩ pull-up resistors are correctly installed (Chapter 04 Step 3).
3. Verify relay VCC is connected to 3.3V (not unconnected or 5V).
4. Verify VCC/JD-VCC jumper is removed (Chapter 04 Step 1).
5. Do not connect load cables until resolved.

> **Note on UVC (Ch5, GPIO 40):** Firmware enforces a 10-second boot guard on the
> UVC channel specifically. Even if other relays are active, UVC will not fire for
> 10 seconds. This is a UV safety feature.

**✓ Check:** Zero relay clicks in the first 5 seconds after power-on.

---

## Test 2 — Relay Click Test

**What this tests:** That each relay physically actuates on command — confirming
3.3V logic is working and the relay coil is good.

If the firmware has a serial relay command interface (check firmware documentation),
trigger each relay by channel number and confirm:
- An audible **mechanical click**
- The relay LED indicator illuminates
- The relay releases cleanly when commanded off

If testing with a simple test sketch before flashing production firmware:

```cpp
const int relayPins[] = {38, 39, 18, 19, 40, 41, 42, 47};

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 8; i++) {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], HIGH); // HIGH = OFF (active LOW)
  }
  delay(5000); // boot lock
}

void loop() {
  for (int i = 0; i < 8; i++) {
    Serial.print("Testing relay ");
    Serial.println(i + 1);
    digitalWrite(relayPins[i], LOW);  // Fire
    delay(500);
    digitalWrite(relayPins[i], HIGH); // Release
    delay(500);
  }
  delay(3000);
}
```

**What you should see:** Each relay produces a clear mechanical click and LED flash.

**If a relay does not click (coil hums or no response):**
1. This indicates 3.3V logic incompatibility — check IN pin resistors (Chapter 04
   Step 2). Replace 1 kΩ resistors with 470 Ω if found.
2. Do not connect load cables to any relay that doesn't fully actuate.

**If a relay fires on multiple channels from one GPIO:**
- Check for solder bridges between adjacent IN pins on the relay board.
- Verify each GPIO wire connects to only one IN pin.

**✓ Check:** All 8 relays click audibly and light their LED indicators. All release
cleanly.

---

## Test 3 — Sensor Detection

**What this tests:** That all sensors are visible on the I2C and 1-Wire buses and
the ADC is reading.

The firmware boot log should show sensor initialisation. Look for:

**Expected I2C addresses:**
| Device | Expected | What to look for in log |
|--------|---------|------------------------|
| TCA9548A | `0x70` | "Found at 0x70" or "TCA9548A OK" |
| SCD30 | `0x61` | "SCD30 found" |
| AS7341 | `0x39` | "AS7341 found" |
| SHT45 #1 via mux ch0 | `0x44` | "SHT45 ch0 found" |
| SHT45 #2 via mux ch1 | `0x44` | "SHT45 ch1 found" |
| SHT45 #3 via mux ch2 | `0x44` | "SHT45 ch2 found" |

**1-Wire:** Should report 5 unique 64-bit ROM addresses.

**ADC (GPIO 7):** Should report a value in millivolts. Exact value depends on how
much water is in the reservoir — a near-zero reading is expected if the sensor is
not submerged.

**If a sensor is not detected:**

*I2C device missing:* Check VIN → 3V3, GND, SDA, SCL wiring for that device.
Run a standalone I2C scan to see what addresses appear:
```cpp
#include <Wire.h>
void setup() {
  Wire.begin(21, 9); // SDA=21, SCL=9 for ESP32-S3
  Serial.begin(115200);
}
void loop() {
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print("Found 0x");
      Serial.println(addr, HEX);
    }
  }
  delay(2000);
}
```
If TCA9548A (`0x70`) is missing, the three SHT45s will also be absent — fix the
multiplexer first. If SCD30 (`0x61`) is missing, check SEL is tied to GND.

*DS18B20 count wrong:* Check the 2.2 kΩ pull-up resistor on GPIO 4. Test with
one probe then add others to isolate which probe has a wiring fault.

*ADC out of range:* Check GPIO 7 wiring and KIT0139 converter board VCC/GND.
Measure voltage at GPIO 7 with a multimeter — should be 0–3.3V.

**✓ Check:** All I2C devices confirmed at expected addresses. 5 DS18B20 ROM
addresses found. ADC reports a value in range.

---

## Test 4 — Failsafe Check

**What this tests:** That MANUAL mode activates the correct relays and no others.

1. Flip MASTER to MANUAL.
2. Close HUMIDITY switch:
   - Ch1 (Fogger) should click and LED lights.
   - Ch2 (Tub fan) should click and LED lights.
   - Ch3–Ch8 should remain off.
3. Open HUMIDITY: Ch1 and Ch2 release.
4. Repeat for each group switch:
   - FAE ON → Ch3 + Ch4 only.
   - UVC ON → Ch5 only.
   - LIGHTS ON → Ch6 only.
5. Return MASTER to AUTO.

**If the wrong relay activates:** Check SPST switch wiring from Chapter 07.
**If no relay activates in MANUAL:** Check DPDT — measure continuity from DPDT
common wire to GND in MANUAL position.

**✓ Check:** Each group switch activates exactly its assigned channels. All others
remain off. AUTO re-engages correctly.

---

## Test 5 — GFCI Test

**What this tests:** That the GFCI/RCD is functional and correctly installed.

1. With the enclosure powered:
2. Press the **TEST** button on the GFCI/RCD.
   - You should hear a click.
   - Power to the enclosure cuts (ESP32 goes dark, relay LEDs off).
3. Press the **RESET** button. Power restores. ESP32 boots again.

**If TEST does not cut power:**
1. Verify the mains input wires connect to the LINE side of the GFCI, not LOAD.
   Reversed LINE/LOAD wiring is the most common installation error.
2. Unplug the enclosure and swap Hot/Neutral at the GFCI LINE terminals if polarity
   was reversed.

**If RESET does not restore power:**
- Confirm the mains input cable is plugged into the wall.
- The RESET button only functions when the GFCI has power on its LINE side.

**✓ Check:** TEST button cuts power. RESET button restores power.

---

## After All 5 Tests Pass

Connect load cables one at a time — each through its PG11 gland and to the correct
relay NO terminal. Tighten each gland. Test each load by triggering the corresponding
relay.

Do not connect load cables until all 5 tests pass.

---

## Chapter 11 Checkpoint

- [ ] All relay NO terminals confirmed empty before first power-on
- [ ] Test 1 passed: no relay clicks in first 5 seconds
- [ ] Test 2 passed: all 8 relays click audibly and LED illuminates
- [ ] Test 3 passed: all sensors detected; ADC reads valid value
- [ ] Test 4 passed: each failsafe group switch activates correct channels only
- [ ] Test 5 passed: GFCI TEST cuts power; RESET restores
- [ ] Load cables connected and each load tested

---

[← Ch 10 — Final Assembly Checklist](10-final-assembly.md)  ·  [Ch 12 — Firmware Flash →](12-firmware-flash.md)
