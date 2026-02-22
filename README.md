# Automated Martha Tent — Build Guides

Two complementary guides for building a fully automated Martha tent fruiting chamber. Start with the physical tent build, then add the DIY controller to replace the off-the-shelf CO2 and humidity controllers with something more accurate and data-rich.

---

## Part 1 — u/dccrens' Martha Tent Build

A proven, fully automated Martha tent fruiting chamber — CO2-controlled FAE, humidity-controlled fogger, modular for easy cleaning. Based on u/dccrens' Martha 2.0 build shared on r/MushroomGrowers in January 2022 (274 upvotes).

**→ [Read the Martha Tent Build Guide](martha-tent-build-guide.md)**

**What you build:** A greenhouse shelving tent with automated CO2 ventilation, external fogger reservoir, cross-flow airflow, and a robust anti-mold water treatment system.

**Cost: ~$250 USD** using off-the-shelf controllers.

---

## Part 2 — DIY ESP32 Controller

Replace the off-the-shelf CO2 and humidity controllers with a single ESP32-based board that gives you more accurate sensors, per-shelf data, a web dashboard, and optional automatic reservoir top-off. Hardware-adapted from u/mettalmag's open-source greenhouse controller (r/MushroomGrowers, Jan 2025).

**→ [Read the DIY Controller Build Guide](diy-controller-build-guide.md)**

**What you build:** An ESP32 controller with Sensirion SCD30 CO2 sensor, SHT45 temp/humidity sensors (×3, one per shelf zone), DS18B20 substrate temperature probes (×5, one per shelf), spectral light monitoring, continuous reservoir water level, and a physical failsafe panel — all in an IP65 enclosure.

**Cost: ~$280–300 USD** — roughly the same price as the off-the-shelf controllers it replaces, with dramatically more capability.

> **Optional add-on:** automatic reservoir top-off pump (pre-wired in base build, ~$20 to activate).

---

## How They Fit Together

The Martha tent guide is self-contained — you can build it exactly as dccrens designed it, using the Inkbird humidity controller and CO2 controller it calls for. The DIY controller guide is a drop-in upgrade that replaces those two controllers (and the Inkbird IBS-TH2 Plus monitor) with the ESP32 board.

The physical tent, fogger system, fans, ducting, and everything else stays the same.

---

## Credits

- **Martha tent build:** u/dccrens, r/MushroomGrowers — https://www.reddit.com/r/MushroomGrowers/comments/sbnlib/
- **DIY controller hardware reference:** u/mettalmag, r/MushroomGrowers — https://www.reddit.com/r/MushroomGrowers/comments/1rao1ms/
- **DIY controller software (coming soon):** based on https://github.com/shonomore/greenhouse-lab-demo
