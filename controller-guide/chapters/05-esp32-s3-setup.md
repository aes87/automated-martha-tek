# Chapter 05 — ESP32-S3 Setup

**What you'll do:** Identify the ESP32-S3 DevKitC-1 board and its pinout, create
a GPIO reference card for this build, bench-test the board before mounting, and
mount it on standoffs in the LV zone.

**Prerequisites:** ESP32-S3 DevKitC-1 board, USB-C cable, computer with
Arduino IDE or PlatformIO, Chapter 02 complete (standoffs installed).

---

## Identifying the ESP32-S3 DevKitC-1

The ESP32-S3 DevKitC-1 is a 38-pin development board. It looks similar to the
original ESP32 DevKit V1 but has some key differences:

- **USB-C connector** (not micro-USB like the V1)
- A "B" button (BOOT) and "EN" button (RESET/ENABLE) near the USB port
- The module on top is labelled "ESP32-S3-WROOM-1"

> **[?] DevKitC-1 vs DevKit V1:** The C in DevKitC-1 stands for the revised
> reference design Espressif released for the S3. The V1 refers to the original
> ESP32 (not S3). They are different chips with different GPIO numbering and
> different pin layouts. This guide is written for the DevKitC-1 with the S3 chip.
> Do not use a DevKit V1 and assume the pin numbers match — they don't.

---

## GPIO Pinout for This Build

Print or write the following table and tape it inside the enclosure lid. You will
refer to it in every remaining wiring chapter.

**ESP32-S3 GPIO assignments — Martha Tent Controller:**

| GPIO | Function | Color (suggest) | Notes |
|------|----------|----------------|-------|
| `GPIO 4` | 1-Wire bus — DS18B20 ×5 | Yellow | 2.2 kΩ pull-up to 3.3V; 100 nF cap at junction |
| `GPIO 7` | ADC — Water level (KIT0139) | Blue | ADC1_CH6; KIT0139 converter provides protection |
| `GPIO 9` | I2C SCL | White | Default SCL in S3 Arduino framework |
| `GPIO 18` | Relay Ch3 — Exhaust fan | Red | Active LOW; safe boot state |
| `GPIO 19` | Relay Ch4 — Intake fan | Red | Active LOW; safe boot state |
| `GPIO 21` | I2C SDA | White | Same as DevKit V1 |
| `GPIO 38` | Relay Ch1 — Fogger | Red | Active LOW; safe boot state |
| `GPIO 39` | Relay Ch2 — Tub fan | Red | Active LOW; safe boot state |
| `GPIO 40` | Relay Ch5 — UVC lights | Red | Active LOW; 10 s boot guard in firmware |
| `GPIO 41` | Relay Ch6 — Grow lights | Red | Active LOW; safe boot state |
| `GPIO 42` | Relay Ch7 — Pump (opt.) | Red | Active LOW; safe boot state |
| `GPIO 47` | Relay Ch8 — Spare | Red | Active LOW; safe boot state |

**Power pins used:**
- `3V3` — 3.3V output — powers sensors and relay VCC (optocoupler side)
- `5V` / `VIN` — 5V input from PSU (or can power board via USB-C)
- `GND` — common ground (multiple pins; use any)

> ⚠️ **SAFETY:** The `3V3` pin is an output from the ESP32's on-board 3.3V
> regulator. Its maximum current output is typically 500–600 mA. The sensors in
> this build draw approximately 100–200 mA total — well within limit. Do not
> connect the 3.3V rail of the PSU to this pin; only draw from it.

---

## Physical Pin Locations on the DevKitC-1

The DevKitC-1 has two rows of 19 pins along each long edge (38 pins total).
Key pins for this build:

```
USB-C end
                ┌──────────────────┐
   GND  ────────┤ GND              │
   3V3  ────────┤ 3V3              │
   RST  ────────┤ RST              │
 GPIO4  ────────┤ 4          47 ├──────── GPIO47
 GPIO5  ────────┤ 5          21 ├──────── GPIO21 (SDA)
 GPIO6  ────────┤ 6          20 ├──────── GPIO20
 GPIO7  ────────┤ 7          19 ├──────── GPIO19
 GPIO8  ────────┤ 8          18 ├──────── GPIO18
 GPIO9  ────────┤ 9          17 ├──────── GPIO17
GPIO10  ────────┤ 10         16 ├──────── GPIO16
GPIO11  ────────┤ 11         15 ├──────── GPIO15
GPIO12  ────────┤ 12         14 ├──────── GPIO14
GPIO13  ────────┤ 13         13 ├──────── GPIO13
                │        ...42  ├──────── GPIO42
                │           41  ├──────── GPIO41
                │           40  ├──────── GPIO40
                │           39  ├──────── GPIO39
                │           38  ├──────── GPIO38
   5V   ────────┤ 5V              │
   GND  ────────┤ GND             │
                └──────────────────┘
                  USB-C end
```

> **Note:** This is a simplified reference. For the authoritative pinout, see the
> official Espressif ESP32-S3-DevKitC-1 pinout PDF from
> [docs.espressif.com](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/esp32-s3-devkitc-1/).

---

## Step 1 — Bench Test Before Mounting

Test the board on your bench before screwing it into the enclosure. A board that
will not enumerate on your computer needs troubleshooting before installation.

**1a — Connect via USB-C:**

1. Plug the ESP32-S3 into your computer with a USB-C cable.
2. On Windows: Open Device Manager. Look for a new COM port under "Ports (COM &
   LPT)" — it should appear as "USB-SERIAL CH340" or "CP210x USB to UART Bridge"
   or "USB Enhanced SERIAL CH342" or similar.
   On macOS/Linux: Run `ls /dev/cu.*` (macOS) or `ls /dev/ttyUSB*` / `ls /dev/ttyACM*`
   (Linux) before and after plugging in to spot the new port.
3. If no port appears: try a different cable (many USB-C cables are charge-only
   with no data lines), a different USB port, or install the CH340 driver for your OS.

> **[?] Why does the S3 still show a UART bridge?** On the DevKitC-1, there is a
> second ESP32-C3 chip (or CH340) on the board that handles USB-to-UART bridging
> for the BOOT/RESET buttons. This is separate from the S3's native USB peripheral
> (which is also exposed on a different port). For flashing via PlatformIO in
> Chapter 12, you'll typically use the UART bridge port. Don't worry about which
> is which yet — PlatformIO handles it.

**1b — Flash a test blink sketch:**

In Arduino IDE (or PlatformIO with a simple test project):

1. Select board: "ESP32S3 Dev Module" (in Arduino IDE: Tools → Board → ESP32
   Arduino → ESP32S3 Dev Module)
2. Select the correct COM port
3. Upload this simple test:

```cpp
void setup() {
  pinMode(2, OUTPUT);    // Onboard LED on GPIO2
  Serial.begin(115200);
  Serial.println("ESP32-S3 alive!");
}

void loop() {
  digitalWrite(2, HIGH);
  delay(500);
  digitalWrite(2, LOW);
  delay(500);
}
```

4. After upload, open Serial Monitor at 115200 baud. You should see:
   `ESP32-S3 alive!`
5. The onboard LED should blink at 1 Hz.

> **[?] GPIO 2 onboard LED:** The DevKitC-1 has a WS2812 RGB LED connected to
> GPIO 48 (not GPIO 2) on some revisions. If the above sketch doesn't blink
> visibly, try GPIO 48 or look for the LED on the board. The Serial output
> confirms the chip is alive regardless.

**✓ Check:** COM port enumerated successfully. Test sketch uploaded. Serial
Monitor shows "ESP32-S3 alive!" Board is functional before mounting.

---

## Step 2 — Mount on Standoffs

1. Position the ESP32-S3 on its standoffs in the LV zone, to the right of the
   relay board's signal header.
2. Orient it so that:
   - The GPIO pins are accessible for wiring
   - The USB-C port faces an edge of the enclosure (so a cable can reach it
     from outside for bench use — though OTA will replace this after firmware flash)
   - The 5V/GND pins that will connect to the PSU are on the side closer to the
     DIN rail
3. Secure with M3 screws — do not overtighten; plastic boards can crack.

**Print or write the GPIO reference card** from the table above and tape it inside
the enclosure lid above the mounting area. Size it to be readable without a
magnifying glass.

**✓ Check:** ESP32-S3 mounted securely on standoffs. GPIO reference card is
attached inside lid. USB-C port is accessible (even if you'll use OTA in future,
it's helpful for initial bring-up in Chapter 12).

---

## Chapter 05 Checkpoint

- [ ] ESP32-S3 DevKitC-1 identified and inspected (no bent pins, no physical damage)
- [ ] COM port enumerated on bench before mounting
- [ ] Test sketch uploaded and serial output confirmed
- [ ] GPIO reference card printed/written and attached to lid
- [ ] Board mounted securely on standoffs in LV zone
- [ ] USB-C port is accessible inside enclosure

---

> **[DevKit V1]** If adapting to a DevKit V1: the board uses a micro-USB port,
> and the GPIO numbers in the reference card above will be wrong. Replace GPIO 38/39
> with 16/17 (with awareness of the UART2 boot glitch), GPIO 40/41/42/47 with
> 23/25/26/27, and GPIO 9 (SCL) with GPIO 22. The pull-up resistors in Chapter 04
> are even more important on the V1 because GPIO 17 glitches LOW at boot. See
> `CLAUDE.md` for the full V1→S3 pin mapping.
