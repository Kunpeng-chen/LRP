# LRP

LRP, LoRa Relay Protocol, is a lightweight relay protocol for private LoRa P2P networks.

The current goal is to land a minimal, testable MVP before adding router, deduplication, radio drivers, or advanced mesh behavior.

LRP is **not LoRaWAN**. It is intended for private LoRa node / relay / base-station systems where the firmware controls both ends of the link.

---

## Current Status

The project currently contains the first MVP frame layer implementation.

Completed:

- Protocol documentation
- MVP implementation plan
- Fixed binary frame format
- Public API headers
- Core frame encode/decode implementation
- CRC16-CCITT validation
- Strict frame validation tests
- CMake build system
- GitHub Actions CI

Current implemented files:

```text
include/lrp_type.h
include/lrp.h
src/lrp.c
tests/test_frame.c
CMakeLists.txt
.github/workflows/ci.yml
```

---

## MVP Scope

The first MVP is intentionally small.

It focuses on proving this path:

```text
Endpoint A → Relay B → Base Station C → ACK → Relay B → Endpoint A
```

The current completed layer only covers:

```text
Frame encode/decode + CRC validation
```

The relay path will be implemented in later steps.

---

## Implemented Frame Layer

The current frame layer supports:

- fixed 13-byte header
- DATA frame
- ACK frame
- 16-bit Network ID
- 16-bit Source Node ID
- 16-bit Destination Node ID
- 16-bit sequence number
- TTL field
- payload length validation
- CRC16-CCITT
- strict decode validation

The protocol version is controlled by the core during encoding:

```c
out[0] = LRP_VERSION;
```

Application code does not control the encoded protocol version.

---

## Frame Layout

```text
Byte 0      version
Byte 1      flags
Byte 2      type
Byte 3      ttl
Byte 4-5    network_id
Byte 6-7    src_id
Byte 8-9    dst_id
Byte 10-11  seq
Byte 12     payload_len
Byte 13..N  payload
Last 2      CRC16
```

All multi-byte fields are encoded in little-endian format.

---

## Public API

```c
#include "lrp.h"
```

Current API:

```c
lrp_status_t lrp_frame_encode(
    const lrp_frame_t *frame,
    uint8_t *out,
    uint16_t out_size,
    uint16_t *out_len
);

lrp_status_t lrp_frame_decode(
    const uint8_t *data,
    uint16_t len,
    lrp_frame_t *out
);

uint16_t lrp_crc16_ccitt(
    const uint8_t *data,
    uint16_t len
);
```

---

## Build and Test

This project uses CMake.

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Expected successful test output includes:

```text
PASS: frame layer strict validation
```

---

## Current Tests

The current strict frame tests cover:

- DATA encode/decode symmetry
- ACK encode/decode with zero payload
- CRC error rejection
- invalid version rejection
- invalid flags rejection
- invalid length rejection
- output buffer too small rejection
- invalid packet type rejection

---

## Documentation

Project documentation:

```text
docs/protocol.md
docs/frame-format.md
docs/relay-algorithm.md
docs/mvp.md
```

These documents define the protocol direction, MVP boundary, frame format, and relay algorithm plan.

---

## Not Implemented Yet

The following features are intentionally not implemented yet:

- duplicate suppression
- router decision logic
- relay forwarding
- end-to-end ACK flow
- random relay delay
- radio abstraction
- SX1262 / SX1276 / LLCC68 integration
- encryption / MIC
- dynamic routing
- Meshtastic compatibility
- LoRaWAN compatibility

---

## Next Steps

Recommended next implementation order:

```text
1. Dedup cache
2. Router decision function
3. Three-node in-memory simulation
4. End-to-end ACK flow
5. Radio abstraction interface
6. Hardware port
```

The next immediate module should be:

```text
dedup
```

because duplicate suppression is required before managed flooding and relay forwarding can be safely implemented.
