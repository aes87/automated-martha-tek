# Chapter 11 — Initial Power-On Test

**What you'll do:** Five structured tests that verify the enclosure is safe and
functional before any load cables are connected. Each test has a "what you should
see" and a "what to do if it fails" section.

**Prerequisites:** Chapter 10 complete (enclosure sealed, all checklists passed).
Computer with serial terminal (115200 baud), USB-C cable for ESP32.

> ⚠️ **SAFETY:** Load cables must be unplugged from the relay NO terminals for this
> entire chapter. The enclosure will be powered, but no mains-connected loads will
> be active. Verify all load cables are disconnected before applying power. This is
> the most important single safety step before first power-on.

---

## Before Power-On: Remove All Load Cables

1. Open the lid (enclosure still unplugged from wall).
2. At the relay board's load terminals, verify that no load cable is connected to
   any NO terminal. The NO terminals should be empty — only the COM terminals have
   wires (from the fuse block).
3. Close the lid.

**✓ Check:** All relay NO terminals confirmed empty. COM terminals have the fuse
block wiring. No load cables are connected.

---

## Setup: Serial Monitor

Before applying power, have your serial monitor ready:

1. Connect the ESP32-S3's USB-C port to your computer.
2. Open Arduino IDE Serial Monitor (or PlatformIO terminal) at **115200 baud**.
3. The serial monitor should show nothing yet (board is not powered from the
   enclosure supply).

> **[?] 115200 baud:** The data rate for serial communication. Baud means bits per
> second. The firmware is configured to output log messages at 115200 baud — if your
> serial monitor is set to a different baud rate (e.g. 9600), you will see garbled
> characters instead of readable text.

---

## Test 1 — Boot-State Check (No Relay Fires at Power-On)

**What this tests:** That the 10 kΩ pull-up resistors and the firmware 5-second boot
lock are working — no relay should fire during the ESP32 startup sequence.

**Procedure:**
1. Plug the enclosure into the wall (or power strip).
2. Listen and watch. In the first 5–10 seconds after power-on:
   - No relay should click
   - No relay LED indicator should light up

3. After 5 seconds, the firmware begins normal operation. You may hear relays click
   as the firmware runs its control loops — this is expected after the boot lock
   expires.

**What you should see:**
- Silence in the first 5 seconds. No relay clicks.
- The GFCI/RCD should not trip.
- The serial monitor begins showing boot log messages.

**What to do if a relay clicks immediately at power-on (before firmware runs):**
1. Unplug the enclosure immediately.
2. The boot-state relay glitch indicates a pull-up resistor problem:
   - Verify the 10 kΩ pull-up resistors are correctly installed on all 8 IN pins
     (Chapter 04).
   - Check that the relay board VCC pin is actually connected to 3.3V (not 5V or
     unconnected).
   - Check that the VCC/JD-VCC jumper is removed (Chapter 04 Step 1).
3. Do not connect load cables until this is resolved.

> **Note on the UVC channel (GPIO 40, Relay Ch5):** The firmware enforces a 10-second
> boot guard specifically for UVC. Even if all other relays are active, UVC will not
> fire for 10 seconds after boot. This is a UV safety feature — a boot glitch on the
> UVC channel could expose someone working near the tent.

**✓ Check:** Zero relay clicks in the first 5 seconds after power-on.

---

## Test 2 — Relay Click Test (Each Relay Activates on Command)

**What this tests:** That each relay physically actuates when commanded — confirming
3.3V logic compatibility and the relay coil is working.

**Procedure:**

If the firmware includes a serial command interface for relay testing:

1. In the serial monitor, send the command to trigger relay 1 (check firmware
   documentation — typically something like `relay 1 on` or `R1`).
2. Listen for an audible **click** from relay Ch1 (fogger channel).
3. The relay LED indicator for Ch1 should illuminate.
4. Send the command to release relay 1. Click again, LED off.
5. Repeat for relays Ch2 through Ch8.

If the firmware does not have a serial relay command yet (pre-flash, running a test
sketch):

Upload a simple relay test sketch in Arduino IDE:
```cpp
const int relayPins[] = {38, 39, 18, 19, 40, 41, 42, 47};

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 8; i++) {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], HIGH); // HIGH = relay OFF (active LOW)
  }
  delay(5000); // boot lock
}

void loop() {
  for (int i = 0; i < 8; i++) {
    Serial.print("Testing relay ");
    Serial.println(i + 1);
    digitalWrite(relayPins[i], LOW);  // Fire relay
    delay(500);
    digitalWrite(relayPins[i], HIGH); // Release relay
    delay(500);
  }
  delay(3000);
}
```

**What you should see:**
- Each relay produces a clear **mechanical click** when triggered.
- The LED indicator on the relay board lights up for that channel.
- The relay clicks off when released.

**What to do if a relay does not click (coil hums or no response):**
1. This indicates a 3.3V logic incompatibility — see Chapter 04 Step 2.
2. Check the IN pin resistors on that relay channel. If they are ~1 kΩ, replace
   with 470 Ω, or replace the relay module with a confirmed 3.3V-compatible one.
3. Do not connect load cables to a relay that does not fully actuate — it may not
   reliably switch under load.

**What to do if a relay fires on two or more channels from one GPIO:**
- Check for solder bridges between adjacent IN pins on the relay board.
- Check that each GPIO wire is connected to only one IN pin.

**✓ Check:** All 8 relays produce audible clicks and LED indicators when triggered.
All relays release cleanly when GPIO returns HIGH.

---

## Test 3 — Sensor Detection

**What this tests:** That all sensors are detected on the I2C and 1-Wire buses,
and that the ADC reads a plausible value from the water level converter.

**Procedure:**
Open the serial monitor. The firmware boot log should show sensor initialisation
messages. Look for:

**I2C scan results (expected addresses):**
| Device | Expected address | What you should see |
|--------|-----------------|---------------------|
| TCA9548A | `0x70` | "I2C device at 0x70" |
| SCD30 | `0x61` | "SCD30 found" or "I2C device at 0x61" |
| AS7341 | `0x39` | "AS7341 found" or "I2C device at 0x39" |
| SHT45 (via mux ch0) | `0x44` | Found on TCA9548A channel 0 |
| SHT45 (via mux ch1) | `0x44` | Found on TCA9548A channel 1 |
| SHT45 (via mux ch2) | `0x44` | Found on TCA9548A channel 2 |

**1-Wire bus:**
- Should report 5 unique 64-bit ROM addresses (one per DS18B20 probe)
- May take 5–10 seconds for the 1-Wire bus to enumerate all probes

**Water level ADC (GPIO 7):**
- Should report a value in millivolts
- With the sensor probe not in water, expect a near-zero reading (close to 4 mA
  minimum current)
- Reasonable range: 200–3100 mV depending on water level

**What to do if a sensor is not detected:**

*I2C device missing:*
1. Check the wiring for that device — VIN, GND, SDA, SCL.
2. Verify VIN is 3V3, not 5V.
3. Run a basic I2C scan sketch to see what addresses appear on the bus:
```cpp
#include <Wire.h>
void setup() {
  Wire.begin(21, 9); // SDA, SCL for ESP32-S3
  Serial.begin(115200);
}
void loop() {
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print("Found device at 0x");
      Serial.println(addr, HEX);
    }
  }
  delay(2000);
}
```
4. If the TCA9548A (`0x70`) is missing, the three SHT45 sensors will also not be
   detected — fix the multiplexer first.
5. If the SCD30 (`0x61`) is missing, check that the SEL pin is tied to GND.

*DS18B20 probes missing or count wrong:*
1. Check the pull-up resistor (2.2 kΩ) is between GPIO 4 and 3V3.
2. Check the decoupling capacitor (100 nF) is at the pull-up junction.
3. Check that all probe data wires (yellow) are on the same GPIO 4 line.
4. Try with just one probe to confirm the bus works, then add probes one at a time.

*ADC reading out of range:*
1. Check the KIT0139 converter board VCC is 3V3 and GND is common GND.
2. Check the OUT wire is connected to GPIO 7 (not a different pin).
3. Measure the voltage at GPIO 7 with a multimeter — should be between 0V and 3.3V.

**✓ Check:** I2C scan confirms `0x70`, `0x61`, `0x39` on main bus. SHT45s detected
on TCA9548A channels 0/1/2 at address `0x44`. 5 unique DS18B20 ROM addresses
enumerated. ADC reports a value in the expected range.

---

## Test 4 — Failsafe Check

**What this tests:** That the MANUAL mode activates the correct relays and that
other relays stay off.

**Procedure:**
1. Flip the MASTER switch from AUTO to MANUAL.
2. Flip the HUMIDITY group switch ON.
   - Relay Ch1 (Fogger) should **click** and LED should light.
   - Relay Ch2 (Tub fan) should **click** and LED should light.
   - Relays Ch3–Ch8 should remain off (no click, no LED).
3. Flip HUMIDITY switch OFF. Ch1 and Ch2 relays should release (click off).
4. Repeat for each group switch:
   - FAE ON: Ch3 (Exhaust) and Ch4 (Intake) click; others stay off.
   - UVC ON: Ch5 clicks; others stay off.
   - LIGHTS ON: Ch6 clicks; others stay off.
5. Flip MASTER back to AUTO.

**What you should see:**
- Each group switch activates exactly its assigned relay channels.
- No other channels activate.
- Returning to AUTO disengages all manual switches.

**What to do if the wrong relay activates:**
- Check the SPST switch wiring from Chapter 07 — verify which IN pins each switch
  connects to.
- Measure continuity from the switch's output terminal to the relay IN pins it
  should control.

**What to do if no relay activates in MANUAL mode:**
- Check the DPDT master: in MANUAL position, confirm continuity between the DPDT
  common wire and GND with a multimeter.
- Verify the SPST switch is properly wired to the DPDT common wire node.

**✓ Check:** Each group switch activates only its assigned channels in MANUAL mode.
All other channels stay off. Auto mode re-engages correctly.

---

## Test 5 — GFCI Test

**What this tests:** That the GFCI/RCD is functional and properly installed.

> ⚠️ **SAFETY:** Do not skip this test. A GFCI that is wired backwards (input to
> LOAD side) will pass the Test button test — because the test circuit is on the
> device itself, not the input wiring. The definitive test is the Reset button
> behaviour: it should not be pressable when the GFCI has no power on the LINE side.

**Procedure:**
1. With the enclosure powered and GFCI/RCD passing power to the PSUs (visible from
   ESP32 being alive):
2. Press the **TEST** button on the GFCI/RCD.
   - You should hear a click from the GFCI.
   - Power to the enclosure internals should cut (ESP32 reboots or goes dark,
     relay LEDs off).
3. Press the **RESET** button.
   - Power should restore.
   - ESP32 boots up again.

**What you should see:**
- TEST button cuts power immediately and audibly.
- RESET button restores power.

**What to do if the TEST button does not cut power:**
1. Verify the mains input wiring goes to the LINE side of the GFCI, not the LOAD side.
   (Reversed LINE/LOAD wiring is the most common installation error.)
2. With the enclosure unplugged, swap the Hot and Neutral wires at the GFCI LINE
   terminals if polarity was reversed.
3. If a DIN-rail RCD: confirm it is wired in series with the mains feed (not in
   parallel).

**What to do if the RESET button does not restore power:**
- Check that the mains input cable is fully plugged into the wall outlet.
- The RESET button only works when the GFCI has power on its LINE side.

**✓ Check:** GFCI/RCD TEST button cuts power. RESET button restores power.

---

## After All 5 Tests Pass

With all tests passed, the enclosure is verified safe and functional with no loads
connected. You may now:

1. Connect load cables one at a time — fogger, fans, UVC, lights — each through
   its PG11 gland and to the correct relay NO terminal.
2. Tighten each load cable gland after the cable is seated.
3. Test each load by triggering the corresponding relay manually or via the
   serial console.

**Do not connect load cables until all 5 tests pass.** If any test fails, resolve
the issue before proceeding.

---

## Chapter 11 Checkpoint

- [ ] All relay NO terminals confirmed empty before first power-on
- [ ] Test 1 passed: no relay clicks in first 5 seconds after power-on
- [ ] Test 2 passed: all 8 relays click audibly and LED illuminates when triggered
- [ ] Test 3 passed: all sensors detected on I2C and 1-Wire; ADC reads valid range
- [ ] Test 4 passed: each failsafe group switch activates only correct relay channels
- [ ] Test 5 passed: GFCI TEST button cuts power; RESET restores
- [ ] (Optional, after all tests) Load cables connected and each load tested
