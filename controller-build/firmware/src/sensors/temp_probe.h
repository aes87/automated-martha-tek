#pragma once
#include <cstdint>
#include <array>
#include <optional>
#include "../../include/config.h"

/**
 * temp_probe.h — DS18B20 substrate temperature probes via 1-Wire bus.
 *
 * Up to DS18B20_PROBE_COUNT (5) probes are discovered at begin() and stored
 * by ROM address. If fewer than 5 are present, the missing entries have
 * valid=false. Probe order is deterministic (ROM address sorted ascending).
 *
 * Hardware notes:
 *   - Pull-up: 2.2 kΩ on GPIO 4 (not the more-common 4.7 kΩ)
 *   - 100 nF decoupling cap at the pull-up junction (close to GPIO 4)
 */
#ifndef NATIVE_TEST
#include <OneWire.h>
#include <DallasTemperature.h>

struct TempProbeReading {
    float    temp_c;
    bool     valid;
    uint32_t timestamp_ms;
    uint8_t  rom[8];  // DS18B20 64-bit ROM address
};

class TempProbes {
public:
    TempProbes() = default;

    /**
     * begin() — Scan 1-Wire bus, store discovered ROM addresses.
     * Returns the number of probes found (0–DS18B20_PROBE_COUNT).
     */
    uint8_t begin();

    /**
     * readAll() — Trigger conversion and read all discovered probes.
     * Blocking: ~750ms at 12-bit resolution. Call from sensor task only.
     */
    std::array<TempProbeReading, DS18B20_PROBE_COUNT> readAll();

    /** probeCount() — Number of probes discovered at begin(). */
    uint8_t probeCount() const { return _count; }

    /** getRom(idx) — 8-byte ROM address for probe idx. */
    const uint8_t* getRom(uint8_t idx) const;

private:
    OneWire         _wire{PIN_ONE_WIRE};
    DallasTemperature _dt{&_wire};
    uint8_t         _roms[DS18B20_PROBE_COUNT][8] = {};
    uint8_t         _count = 0;
};

extern TempProbes TempProbeArray;
#endif
