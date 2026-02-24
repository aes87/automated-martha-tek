# Chapter 12 — Firmware Flash

**What you'll do:** Install PlatformIO, clone the firmware repository, flash the
ESP32-S3 with the production firmware, verify sensor readings in the serial output,
access the web dashboard, and configure OTA for future updates.

**Prerequisites:** Chapter 11 complete (all 5 power-on tests passed). Computer
with internet access, USB-C cable for ESP32.

---

## Step 1 — Install PlatformIO

PlatformIO is a development environment that handles compiler toolchains, library
management, and device flashing. The easiest way to use it is through its VS Code
extension.

**Option A — PlatformIO VS Code extension (recommended):**

1. Install [Visual Studio Code](https://code.visualstudio.com/) if not already
   installed.
2. Open VS Code. Go to Extensions (Ctrl+Shift+X or Cmd+Shift+X).
3. Search for "PlatformIO IDE" and install it.
4. Restart VS Code. PlatformIO will install its core components on first launch.

**Option B — PlatformIO CLI (advanced):**

Install the PlatformIO CLI via pip:
```bash
pip install platformio
```

Both options use the same `pio` commands under the hood.

> **[?] PlatformIO vs Arduino IDE:** Both can program the ESP32. PlatformIO is
> preferred here because it uses a `platformio.ini` configuration file that
> specifies the exact board, framework version, and libraries. When you clone the
> firmware repo, PlatformIO will automatically download the correct versions of
> everything. With the Arduino IDE, you'd need to manually install board support
> and library versions.

---

## Step 2 — Clone the Firmware Repository

1. Open a terminal (or VS Code's integrated terminal).
2. Clone the firmware repository:

```bash
git clone https://github.com/aes87/automated-martha-tek.git
cd automated-martha-tek/firmware
```

3. Open the `firmware/` folder in VS Code: File → Open Folder → select the
   `firmware` directory.
4. PlatformIO will detect the `platformio.ini` file and prompt to install
   dependencies. Click "Yes" or run:

```bash
pio pkg install
```

---

## Step 3 — Understand the `platformio.ini` Environments

The firmware supports two board variants. Open `platformio.ini` and you will see
two environments:

```ini
[env:esp32s3]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino
build_flags = -DBOARD_S3
...

[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
; no BOARD_S3 flag — uses DevKit V1 pin assignments
...
```

> **[?] -DBOARD_S3 build flag:** This defines the `BOARD_S3` preprocessor symbol
> when compiling. The `hal.h` header file checks for this symbol and applies the
> S3 pin remapping (GPIO 38/39 instead of 16/17, GPIO 9 instead of 22 for SCL,
> etc.). If you build with the `esp32dev` environment, the V1 pin numbers are used.
> Always use `esp32s3` for this build.

**You will use the `esp32s3` environment for this build.**

---

## Step 4 — First Flash

**4a — Connect the ESP32-S3:**

1. Connect the USB-C cable between the ESP32-S3 and your computer.
2. The board should enumerate (confirm in Device Manager or `ls /dev/tty*`).

**4b — Flash the firmware:**

In VS Code with PlatformIO: click the "Upload" arrow button in the PlatformIO
toolbar (or Ctrl+Alt+U).

Via CLI:
```bash
pio run -e esp32s3 --target upload
```

PlatformIO will:
1. Compile the firmware (may take 2–5 minutes on first build)
2. Detect the COM port
3. Put the ESP32-S3 into programming mode automatically (via DTR/RTS lines)
4. Flash the firmware
5. Reset the board

**What you should see:**
```
Compiling...
Linking...
Building .pio/build/esp32s3/firmware.bin
Uploading...
Writing at 0x00001000... (2%)
...
Writing at 0x000c0000... (100%)
Hash of data verified.
Leaving...
Hard resetting via RTS pin...
```

**What to do if upload fails:**

*"No such device" or "Port not found":*
- Check USB-C cable is data-capable (not charge-only)
- Try a different USB port
- On Linux, add your user to the `dialout` group: `sudo usermod -a -G dialout $USER`

*"Permission denied" (Linux/macOS):*
- `sudo chmod a+rw /dev/ttyACM0` (replace with your port)

*Board not entering programming mode automatically:*
- Hold the BOOT button on the ESP32-S3, press and release RESET, then release BOOT
- This manually puts the board into download mode

---

## Step 5 — Serial Monitor: First Boot Log Walk-Through

Immediately after flashing, open the serial monitor:

**PlatformIO:**
```bash
pio device monitor --baud 115200 --environment esp32s3
```

**VS Code:** Click the plug icon in the PlatformIO toolbar.

**Expected boot log sequence:**

```
Martha Tent Controller v0.1.0
Board: ESP32-S3 DevKitC-1
Boot #1 | Reset reason: Power-on

[HAL] Pin table: FOGGER=38 TUB_FAN=39 EXHAUST=18 INTAKE=19 UVC=40 LIGHTS=41 PUMP=42 SPARE=47
[HAL] I2C: SDA=21 SCL=9 | 1-Wire: GPIO=4 | ADC water: GPIO=7

[RELAY] Boot lock active — all relays held OFF for 5000ms
[RELAY] UVC extra guard — held OFF for additional 5000ms (10s total)

[I2C] Scanning bus...
[I2C] Found: 0x39 (AS7341)
[I2C] Found: 0x61 (SCD30)
[I2C] Found: 0x70 (TCA9548A)
[I2C] Scan complete — 3 devices

[TCA] Probing channel 0: found SHT45 at 0x44
[TCA] Probing channel 1: found SHT45 at 0x44
[TCA] Probing channel 2: found SHT45 at 0x44

[1W] Bus scan... found 5 devices:
  ROM[0]: 28XXXXXXXXXXXX01
  ROM[1]: 28XXXXXXXXXXXX02
  ROM[2]: 28XXXXXXXXXXXX03
  ROM[3]: 28XXXXXXXXXXXX04
  ROM[4]: 28XXXXXXXXXXXX05

[ADC] Water level: 412 mV (calibration required — set MIN/MAX in web UI)

[WiFi] Connecting to <your SSID>...
[WiFi] Connected. IP: 192.168.1.xxx
[mDNS] Hostname: martha.local

[RELAY] Boot lock expired — relay control active
Martha ready.
```

The log message text may vary from the actual firmware output — what matters is that
you see:
- `Board: ESP32-S3 DevKitC-1` (confirms BOARD_S3 flag is active)
- All sensors detected (AS7341, SCD30, TCA9548A, 3× SHT45, 5× DS18B20)
- IP address assigned

**What to do if the log shows wrong pin numbers:**
If you see `SCL=22` instead of `SCL=9`, or GPIO 16/17 relay pins, the firmware was
built with the wrong environment (`esp32dev` instead of `esp32s3`). Rebuild and
reflash with `pio run -e esp32s3 --target upload`.

---

## Step 6 — Sensor Reading Verification

After a minute or two for sensors to warm up, the serial output should begin
showing live readings:

```
[SHT45-0] T: 22.4°C  RH: 54.2%  VPD: 1.26 kPa  (top shelf)
[SHT45-1] T: 22.1°C  RH: 55.0%  VPD: 1.22 kPa  (mid shelf)
[SHT45-2] T: 21.8°C  RH: 56.1%  VPD: 1.18 kPa  (bottom shelf)

[SCD30]   CO2: 412 ppm  T: 22.3°C  RH: 54.8%

[DS18B20] Shelf1: 22.1°C  Shelf2: 22.0°C  Shelf3: 21.9°C
          Shelf4: 21.8°C  Shelf5: 21.7°C

[AS7341]  Clear: 1240  F1(415nm): 210  F2(445nm): 380  ...

[WATER]   Level: 68%  Raw: 2188 mV
```

**Sanity checks for this bench test (sensors not yet in tent):**
- Temperature: Should read room temperature ± 1°C
- RH: Should read room humidity (if you have a reference, compare)
- CO2: Should read outdoor ambient (~420 ppm) to indoor ambient (~500–1000 ppm)
- DS18B20: All 5 should read similar values (they're all at room temperature)
- Water level: A raw mV value is fine — calibration happens in the web UI

**What to do if a sensor shows an error or missing data:**
- Refer to Chapter 11, Test 3 troubleshooting steps — same root causes apply.

---

## Step 7 — Web Dashboard

With WiFi connected, the controller hosts a live web dashboard:

1. On the same network as the controller, open a browser and navigate to:
   `http://martha.local`

   If mDNS doesn't resolve (common on some Android devices or networks with mDNS
   filtering), use the IP address shown in the serial log:
   `http://192.168.1.xxx` (replace with your IP)

2. The dashboard should show:
   - CO2 reading with trend graph
   - Temperature and RH per shelf height
   - VPD per shelf
   - Substrate temperature per shelf
   - Water level with percentage and fill graph
   - Relay status (on/off) for all 8 channels
   - Light spectrum data

3. Navigate to Settings to:
   - Set water level sensor MIN and MAX calibration values (measured in mV at
     empty and full reservoir)
   - Set WiFi credentials if currently using fallback AP
   - Configure humidity setpoint and CO2 thresholds (firmware defaults are
     RH 85%, CO2 950 ppm)

> ⚠️ **SAFETY:** Do not expose the web dashboard to the internet. This controller
> has physical relay outputs — remote access from outside your home network is a
> fire and safety risk. Keep it on your local WiFi only. Do not set up port
> forwarding to this device.

---

## Step 8 — Configure OTA for Future Updates

OTA (Over-The-Air) updates let you flash new firmware without opening the enclosure
or connecting a USB cable.

1. In the web dashboard Settings, enable OTA and set an OTA password.
2. PlatformIO can push updates over the network using the OTA IP or `martha.local`:

```bash
pio run -e esp32s3 --target upload --upload-port martha.local
```

3. Enter the OTA password when prompted.

With OTA configured, the enclosure should not need to be opened again for firmware
updates. This is important because every time you open a sealed IP65 enclosure,
you risk disturbing wiring, displacing the gasket, or letting humidity into the
electronics.

> **[?] OTA security:** OTA updates over plaintext HTTP are vulnerable to
> man-in-the-middle attacks on your network. If you use OTA on a trusted home
> network, the risk is low. The firmware should use HTTPS OTA for stronger
> protection — check the firmware documentation for its OTA security implementation.

---

## Chapter 12 Checkpoint

- [ ] PlatformIO installed (VS Code extension or CLI)
- [ ] Firmware repo cloned successfully
- [ ] `platformio.ini` environments reviewed; `esp32s3` environment confirmed
- [ ] Firmware compiled without errors using `esp32s3` environment
- [ ] Firmware flashed successfully to ESP32-S3
- [ ] Serial monitor open at 115200 baud
- [ ] Boot log shows `Board: ESP32-S3 DevKitC-1` and correct GPIO pin table
- [ ] All sensors detected in boot log (TCA9548A, SCD30, AS7341, 3×SHT45, 5×DS18B20)
- [ ] Sensor readings appear in serial output with plausible values
- [ ] Web dashboard accessible at `http://martha.local` or IP address
- [ ] Water level calibration MIN/MAX values set in Settings
- [ ] OTA configured for future firmware updates

---

## You're Done

The controller is built, tested, and running firmware. The remaining steps before
a grow:

1. Mount sensors in the tent (see `setup/` documentation for placement guide)
2. Connect load cables to the tent equipment (fogger, fans, UVC, grow lights)
3. Do an initial grow environment test run: verify automation triggers at the
   correct setpoints (85% RH, 950 ppm CO2)
4. Calibrate the water level sensor with the reservoir filled and empty
5. Label the reservoir sensor and set the auto-refill thresholds (if using the
   optional pump)

Refer to the [web dashboard] Settings page and the firmware documentation for
control loop tuning.
