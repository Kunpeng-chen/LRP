# LRP

LRP, LoRa Relay Protocol, is a lightweight relay protocol stack for private LoRa P2P networks.

It is **not LoRaWAN**. LRP is designed for systems where endpoint, relay, and base-station firmware are all controlled by the application developer.

The current repository contains a complete MVP relay core:

```text
Endpoint A → Relay B → Base Station C → ACK → Relay B → Endpoint A
```

---

## Features

- Fixed binary frame format
- DATA and ACK frames
- CRC16-CCITT validation
- Duplicate suppression
- Relay decision logic: `DROP`, `CONSUME`, `FORWARD`
- TTL decrement on forwarding
- Three-node in-memory relay simulation
- Basic usage sample
- CMake build
- CTest unit tests
- GitHub Actions CI

---

## Architecture

LRP uses a small layered protocol-stack design:

```text
Application / Sample
    ↓
Router Decision Layer
    ↓
Dedup Layer
    ↓
Frame Layer
    ↓
Future Radio / Port Layer
```

Users normally only need:

```c
#include "lrp.h"
```

---

## Frame Format

```text
[13B Header] + [0~64B Payload] + [2B CRC16]
```

Header layout:

```text
version | flags | type | ttl | network_id | src_id | dst_id | seq | payload_len
```

All multi-byte fields use little-endian encoding.

---

## Build

```bash
cmake -S . -B build
cmake --build build
```

---

## Run Tests

```bash
ctest --test-dir build --output-on-failure
```

---

## Run Simulation

```bash
./build/simulated_three_nodes
```

Expected result:

```text
PASS: simulated three-node relay path
```

---

## Run Sample

```bash
./build/sample_mvp_basic_usage
```

Expected result:

```text
PASS: MVP basic usage sample
```

---

## Project Layout

```text
include/    Public headers
src/        Protocol stack implementation
tests/      Unit tests
examples/   End-to-end relay simulation
samples/    User-facing usage sample
docs/       Design documents
```

---

## Documentation

Detailed design documents are available in:

```text
docs/protocol.md
docs/frame-format.md
docs/relay-algorithm.md
docs/mvp.md
```

---

## Not Implemented Yet

- Real LoRa radio integration
- Radio abstraction layer
- Randomized relay delay
- Encryption / MIC
- Dynamic routing
- LoRaWAN compatibility
