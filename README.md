# LRP

LRP, LoRa Relay Protocol, is a lightweight relay protocol stack for private LoRa P2P networks.

LRP is **not LoRaWAN**. It is intended for private LoRa node / relay / base-station systems where the firmware controls both ends of the link.

The current repository contains a complete MVP relay protocol core with frame handling, duplicate suppression, relay decision logic, in-memory relay simulation, sample usage, tests, and CI.

---

## Current MVP Status

```text
Frame Layer        DONE
Dedup Layer        DONE
Router Layer       DONE
Relay Simulation   DONE
Basic Sample       DONE
CMake Build        DONE
CI                 DONE
```

The MVP proves this path:

```text
Endpoint A → Relay B → Base Station C → ACK → Relay B → Endpoint A
```

---

## Software Architecture

LRP now follows a layered protocol stack architecture:

```text
+-----------------------------+
| Application / Sample        |
+-----------------------------+
| Router Decision Layer       |
+-----------------------------+
| Dedup Layer                 |
+-----------------------------+
| Frame Layer                 |
+-----------------------------+
| Future Radio / Port Layer   |
+-----------------------------+
```

The current stack is synchronous, lightweight, and suitable for embedded C projects. It does not use dynamic memory, threads, message queues, or an RTOS.

---

## Layer Communication Model

Layers communicate through:

1. `lrp_frame_t` as the shared packet object.
2. Function calls as layer boundaries.
3. Return values as control signals.
4. Router-owned state for deduplication.

Receive flow:

```text
Radio bytes
    ↓
lrp_frame_decode()
    ↓
lrp_frame_t
    ↓
lrp_router_on_frame()
    ↓
DROP / CONSUME / FORWARD
```

Forward flow:

```text
FORWARD decision
    ↓
out frame with TTL - 1
    ↓
lrp_frame_encode()
    ↓
Radio bytes
```

Current radio bytes are simulated in memory. A real radio layer will be added later.

---

## Implemented Layers

### Frame Layer

Files:

```text
include/lrp_frame.h
src/lrp_frame.c
```

Responsibilities:

- fixed binary frame format
- DATA frame
- ACK frame
- payload length validation
- CRC16-CCITT
- strict decode validation
- little-endian encoding

### Dedup Layer

Files:

```text
include/lrp_dedup.h
src/lrp_dedup.c
```

Responsibilities:

- duplicate suppression
- cache expiration
- managed flooding protection

Dedup key:

```text
network_id + src_id + seq + type
```

### Router Decision Layer

Files:

```text
include/lrp_router.h
src/lrp_router.c
```

Responsibilities:

- network filtering
- duplicate filtering
- destination matching
- relay enable / disable
- TTL check
- TTL decrement on forwarding

Router decisions:

```text
DROP
CONSUME
FORWARD
```

---

## Public Headers

```text
include/lrp.h          umbrella public API
include/lrp_type.h     shared constants and types
include/lrp_frame.h    frame layer API
include/lrp_dedup.h    dedup layer API
include/lrp_router.h   router layer API
```

Users can include only:

```c
#include "lrp.h"
```

---

## Source Layout

```text
src/lrp_frame.c
src/lrp_dedup.c
src/lrp_router.c
```

Each source file maps to one protocol stack layer.

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

Frame size:

```text
Minimum: 15 bytes
Maximum: 79 bytes
```

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

## Run Basic Usage Sample

After build:

```bash
./build/sample_mvp_basic_usage
```

This sample demonstrates:

- application frame creation
- frame encode/decode
- relay forwarding
- ACK generation
- endpoint ACK receive flow

Expected output:

```text
Endpoint creates DATA
Relay forwards DATA
Base consumes DATA
Base creates ACK
Relay forwards ACK
Endpoint receives ACK
PASS: MVP basic usage sample
```

---

## Repository Structure

```text
include/
  lrp.h
  lrp_type.h
  lrp_frame.h
  lrp_dedup.h
  lrp_router.h

src/
  lrp_frame.c
  lrp_dedup.c
  lrp_router.c

examples/
  simulated_three_nodes/main.c

samples/
  mvp_basic_usage/main.c

tests/
  test_frame.c
  test_dedup.c
  test_router.c

docs/
  protocol.md
  frame-format.md
  relay-algorithm.md
  mvp.md

.github/workflows/
  ci.yml
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

## Next Architecture Step

The next stack layer should be:

```text
Radio / Port Layer
```

Expected future structure:

```text
Application
    ↓
LRP Router
    ↓
LRP Dedup
    ↓
LRP Frame
    ↓
LRP Radio Abstraction
    ↓
SX1262 / SX1276 / LLCC68 Driver
```
