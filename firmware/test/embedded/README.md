# Hardware Bring-Up Checklist

Run this checklist when flashing to hardware for the first time (Phase 2 smoke test).
Use the serial monitor at 115200 baud.

## Before First Power-On

- [ ] All relay IN pins have 10 kΩ pull-ups to relay VCC rail (C2 fix)
- [ ] GPIO 34 has 1 kΩ series + 3.3V Zener/TVS protection (C3 fix)
- [ ] 3.3V-compatible relay module OR 470 Ω resistors on relay IN pins (C1 fix)
- [ ] GFCI/RCD installed as first device on mains feed (C4 fix)
- [ ] **No mains loads connected** — use dummy 12V DC loads for initial relay test

## Boot Verification

- [ ] Serial shows: `[INFO][main    ][...] Martha Tent Controller vX.X.X booting`
- [ ] Serial shows: `[INFO][main    ][...] I2C init SDA=21 SCL=22`
- [ ] No relay clicks during first 5 seconds (boot lock)
- [ ] Serial shows: `[INFO][main    ][...] Boot complete`

## I2C Scan

- [ ] TCA9548A found at 0x70
- [ ] SCD30 found at 0x61
- [ ] AS7341 found at 0x39
- [ ] SHT45 ×3 each found at 0x44 (via mux channels 0/1/2)

## 1-Wire Scan

- [ ] 5 DS18B20 ROM addresses discovered and printed
- [ ] Temperature readings in plausible range (15–35°C)

## Sensor Readings

After 10 seconds, serial sensor output should show:
- [ ] CO2: 400–500 ppm (ambient, before adding any mushroom blocks)
- [ ] RH: within ±5% of a reference hygrometer
- [ ] All substrate temps within ±1°C of ambient
- [ ] Water level: percentage corresponding to known fill level
- [ ] Light: non-zero values across channels

## Relay Test (with dummy 12V loads)

- [ ] Open `http://martha.local` in browser
- [ ] Enter Manual Mode
- [ ] Toggle each relay — confirm physical relay clicks and LED indicator on relay board lights
- [ ] Exit Manual Mode — confirm control loops resume
- [ ] No relay fires during the 5s UVC boot guard after reset

## Network

- [ ] `http://martha.local` resolves on local network
- [ ] WebSocket updates visible on dashboard (sensor values updating every 2s)
- [ ] `GET /api/status` returns valid JSON
- [ ] `GET /api/config` returns current thresholds
- [ ] OTA update page accessible at `http://martha.local/update`

## Watchdog

- [ ] Normal operation: no watchdog resets (check via `/api/status` boot_reason field)
- [ ] Suspend sensor task (comment out vTaskDelay) → confirm reset after 30s
- [ ] Config survives reset: RH threshold set to 87%, device reset, verify 87% in `/api/config`
