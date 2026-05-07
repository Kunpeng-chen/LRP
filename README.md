# LRP

LRP, LoRa Relay Protocol, is a lightweight relay protocol for private LoRa P2P networks.

The current goal is to land a minimal, testable MVP before adding router, deduplication, radio drivers, or advanced mesh behavior.

LRP is **not LoRaWAN**. It is intended for private LoRa node / relay / base-station systems where the firmware controls both ends of the link.

---

## Current Status

The project currently contains a working MVP relay core.

Completed:

- Protocol documentation
- MVP implementation plan
- Fixed binary frame format
- Public API headers
- Frame encode/decode implementation
- CRC16-CCITT validation
- Deduplication cache
- Router decision layer
- Three-node relay simulation
- Strict unit tests
- CMake build system
- GitHub Actions CI

Current implemented files:

```text
include/lrp_type.h
include/lrp.h
src/lrp.c

examples/simulated_three_nodes/main.c

tests/test_frame.c
tests/test_dedup.c
tests/test_router.c

CMakeLists.txt
.github/workflows/ci.yml
```

---

## MVP Scope

The first MVP proves this relay path:

```text
Endpoint A → Relay B → Base Station C → ACK → Relay B → Endpoint A
```

The project now contains an in-memory simulation of this complete path.

---

## Implemented Layers

### Frame Layer

Implemented:

- fixed 13-byte header
- DATA frame
- ACK frame
- payload validation
- CRC16-CCITT
- strict decode validation
- little-endian encoding

### Dedup Layer

Implemented:

- duplicate suppression cache
- timeout-based expiration
- managed flooding protection

Dedup key:

```text
network_id + src_id + seq + type
```

### Router Decision Layer

Implemented decisions:

```text
DROP
CONSUME
FORWARD
```

Router supports:

- network filtering
- duplicate filtering
- relay enable/disable
- TTL decrement on forwarding
- destination matching

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

## Build

This project uses CMake.

```bash
cmake -S . -B build
cmake --build build
```

---

## Run Tests

```bash
ctest --test-dir build --output-on-failure
```

Current test targets:

```text
frame_layer_strict_validation
dedup_layer_strict_validation
router_layer_strict_validation
```

---

## Run MVP Relay Simulation

After build:

```bash
./build/simulated_three_nodes
```

Expected output:

```text
Endpoint A sends DATA
Relay B forwards DATA
Base C consumes DATA
Base C sends ACK
Relay B forwards ACK
Endpoint A receives ACK
PASS: simulated three-node relay path
```

---

## Documentation

Project documentation:

```text
docs/protocol.md
docs/frame-format.md
docs/relay-algorithm.md
docs/mvp.md
```

---

## Not Implemented Yet

The following features are intentionally not implemented yet:

- randomized relay delay
- radio abstraction
- SX1262 / SX1276 / LLCC68 integration
- encryption / MIC
- dynamic routing
- mesh routing metrics
- Meshtastic compatibility
- LoRaWAN compatibility
- real RF transmission

---

## Current MVP Status

Current status:

```text
Frame Layer        DONE
Dedup Layer        DONE
Router Layer       DONE
Relay Simulation   DONE
CI                 DONE
```

The repository now contains a complete minimal relay protocol MVP core.
