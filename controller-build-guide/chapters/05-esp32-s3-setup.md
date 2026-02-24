# Chapter 05 — ESP32-S3 Setup

**What you'll do:** Identify the ESP32-S3 DevKitC-1 board and its pinout, create
a GPIO reference card for this build, bench-test the board before mounting, and
mount it on standoffs in the LV zone.

**Prerequisites:** ESP32-S3 DevKitC-1 board, USB-C cable, computer with Arduino
IDE or PlatformIO, Chapter 02 complete (standoffs installed).

---

## Identifying the ESP32-S3 DevKitC-1

The ESP32-S3 DevKitC-1 is a 38-pin development board with:
- **USB-C connector** (not micro-USB)
- BOOT and EN (reset) buttons near the USB port
- Module on top labelled "ESP32-S3-WROOM-1"

<details>
<summary><strong>[?] DevKitC-1:</strong></summary>

The reference development board Espressif produces for the
ESP32-S3 chip. The "C" revision introduced USB-C. All GPIO numbers and wiring in
this guide assume the 38-pin DevKitC-1.

</details>

---

## GPIO Pinout for This Build

Print or write the following table and tape it inside the enclosure lid. You will
refer to it in every remaining wiring chapter.

**ESP32-S3 GPIO assignments — Martha Tent Controller:**

| GPIO | Function | Notes |
|------|----------|-------|
| `GPIO 4` | 1-Wire bus — DS18B20 ×5 | 2.2 kΩ pull-up to 3V3; 100 nF cap at junction |
| `GPIO 7` | ADC — Water level (KIT0139) | ADC1_CH6; KIT0139 converter provides protection |
| `GPIO 9` | I2C SCL | Default SCL in S3 Arduino framework |
| `GPIO 18` | Relay Ch3 — Exhaust fan | Active LOW; safe boot state |
| `GPIO 19` | Relay Ch4 — Intake fan | Active LOW; safe boot state |
| `GPIO 21` | I2C SDA | |
| `GPIO 38` | Relay Ch1 — Fogger | Active LOW; safe boot state |
| `GPIO 39` | Relay Ch2 — Tub fan | Active LOW; safe boot state |
| `GPIO 40` | Relay Ch5 — UVC lights | Active LOW; 10 s boot guard in firmware |
| `GPIO 41` | Relay Ch6 — Grow lights | Active LOW; safe boot state |
| `GPIO 42` | Relay Ch7 — Pump (opt.) | Active LOW; safe boot state |
| `GPIO 47` | Relay Ch8 — Spare | Active LOW; safe boot state |

**Power pins used:**
- `3V3` — 3.3V output — powers sensors and relay VCC (optocoupler side)
- `5V` / `VIN` — 5V input from PSU
- `GND` — common ground (multiple GND pins; use any)

> [!WARNING]
> The `3V3` pin is an output from the ESP32's on-board regulator, rated ~500–600 mA.
> Do not connect the 3.3V PSU rail to this pin.

---

## Physical Pin Locations on the DevKitC-1

The board has two rows of 19 pins. Key pins for this build (simplified):

```
USB-C end
                ┌──────────────────┐
   GND  ────────┤ GND              │
   3V3  ────────┤ 3V3              │
 GPIO4  ────────┤ 4          47 ├──────── GPIO47
 GPIO7  ────────┤ 7          21 ├──────── GPIO21 (SDA)
 GPIO9  ────────┤ 9          19 ├──────── GPIO19
                │           18 ├──────── GPIO18
                │        ...42  ├──────── GPIO42
                │           41  ├──────── GPIO41
                │           40  ├──────── GPIO40
                │           39  ├──────── GPIO39
                │           38  ├──────── GPIO38
   5V   ────────┤ 5V              │
   GND  ────────┤ GND             │
                └──────────────────┘
```

For the authoritative pinout, refer to the Espressif ESP32-S3-DevKitC-1 datasheet.

---

## Step 1 — Bench Test Before Mounting

Test the board on your bench before screwing it into the enclosure.

**1a — Connect via USB-C:**

1. Plug the ESP32-S3 into your computer with a USB-C cable.
2. Confirm a new COM port or `/dev/tty*` device appears:
   - Windows: Device Manager → Ports (COM & LPT)
   - macOS: `ls /dev/cu.*` before/after plugging in
   - Linux: `ls /dev/ttyUSB*` or `ls /dev/ttyACM*`
3. If no port appears: try a data-capable USB-C cable; install the CH340 driver
   for your OS if needed.

**1b — Flash a test blink sketch:**

```cpp
void setup() {
  pinMode(2, OUTPUT);
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

After upload, open Serial Monitor at 115200 baud. You should see:
```
ESP32-S3 alive!
```

**✓ Check:** COM port enumerated. Test sketch uploaded. Serial Monitor shows
"ESP32-S3 alive!" Board is functional before mounting.

---

## Step 2 — Mount on Standoffs

1. Position the ESP32-S3 on its standoffs in the LV zone, to the right of the
   relay board's signal header.
2. Orient so the USB-C port faces an accessible edge and GPIO pins are reachable
   for wiring.
3. Secure with M3 screws. Do not overtighten.

**Print the GPIO reference card** from the table above and tape it inside the
enclosure lid.

**✓ Check:** ESP32-S3 mounted securely. GPIO reference card attached inside lid.

---

## Chapter 05 Checkpoint

- [ ] Board identified and physically inspected (no bent pins, no damage)
- [ ] COM port enumerated on bench
- [ ] Test sketch uploaded and serial output confirmed
- [ ] GPIO reference card printed and attached to lid interior
- [ ] Board mounted on standoffs in LV zone
- [ ] USB-C port accessible inside enclosure

---

[← Ch 04 — Relay Module Setup](04-relay-module-setup.md)  ·  [Ch 06 — Sensor Wiring →](06-sensor-wiring.md)
