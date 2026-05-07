# LRP MVP Implementation Plan

This document defines the first minimal implementable version of LRP.

The goal of the MVP is not to build a full mesh network. The goal is to prove that a private LoRa node can send a small packet to a base station through one relay using a simple, deterministic, and testable protocol core.

---

# 1. MVP Goal

The MVP must support this topology:

```text
Endpoint A  →  Relay B  →  Base Station C
```

Minimum success condition:

```text
Endpoint sends DATA
Relay validates and forwards DATA
Base Station receives DATA
Base Station sends ACK
Relay forwards ACK
Endpoint receives ACK
```

---

# 2. MVP Scope

The MVP includes only:

1. Fixed binary frame format.
2. DATA frame.
3. ACK frame.
4. CRC16 validation.
5. 16-bit Network ID.
6. 16-bit Node ID.
7. 16-bit sequence number.
8. TTL.
9. Duplicate cache.
10. Random relay delay.
11. End-to-end ACK.
12. Pure C protocol core.
13. Radio abstraction interface.
14. Unit tests for frame parsing and relay decision logic.

---

# 3. Explicitly Out of Scope

The MVP does not include:

1. LoRaWAN compatibility.
2. Dynamic routing.
3. Route discovery.
4. Neighbor table.
5. Encryption.
6. MIC.
7. Fragmentation.
8. OTA configuration.
9. Low-power wake-on-radio relay.
10. Multi-channel support.
11. Serial transparent stream mode.
12. Large payload support.
13. Full Meshtastic compatibility.

These may be added later after the one-relay path is stable.

---

# 4. Fixed MVP Frame

The MVP frame is:

```text
+------------+------+-------------+
| Header     | Data | CRC16       |
+------------+------+-------------+
```

Header:

| Field | Size |
| --- | --- |
| version | 1 byte |
| flags | 1 byte |
| type | 1 byte |
| ttl | 1 byte |
| network_id | 2 bytes |
| src_id | 2 bytes |
| dst_id | 2 bytes |
| seq | 2 bytes |
| payload_len | 1 byte |

Header size:

```text
13 bytes
```

CRC16 is calculated over:

```text
header + payload
```

---

# 5. MVP Constants

```c
#define LRP_VERSION              0x01
#define LRP_MAX_PAYLOAD_SIZE     64
#define LRP_HEADER_SIZE          13
#define LRP_CRC_SIZE             2
#define LRP_MAX_FRAME_SIZE       (LRP_HEADER_SIZE + LRP_MAX_PAYLOAD_SIZE + LRP_CRC_SIZE)

#define LRP_TYPE_DATA            0x01
#define LRP_TYPE_ACK             0x02

#define LRP_FLAG_ACK_REQUIRED    0x01
#define LRP_FLAG_IS_ACK          0x02

#define LRP_ADDR_INVALID         0x0000
#define LRP_ADDR_BASE            0x0001
#define LRP_ADDR_BROADCAST       0xFFFF
```

---

# 6. MVP Runtime Roles

## 6.1 Endpoint

Endpoint behavior:

1. Build DATA frame.
2. Set `src_id = endpoint_id`.
3. Set `dst_id = base_id`.
4. Set `ttl = 1` for one relay.
5. Set `ACK_REQUIRED` when reliable delivery is needed.
6. Send frame.
7. Wait for ACK.
8. Retry at most 1 to 3 times.

## 6.2 Relay

Relay behavior:

1. Receive frame.
2. Decode frame.
3. Verify CRC.
4. Verify Network ID.
5. Check duplicate cache.
6. If frame is addressed to relay, consume it.
7. If TTL is zero, drop it.
8. Decrement TTL.
9. Apply random delay.
10. Forward frame.

## 6.3 Base Station

Base station behavior:

1. Receive DATA frame.
2. Decode and verify it.
3. If addressed to base station, pass payload to application.
4. If `ACK_REQUIRED` is set, send ACK frame back to source.

---

# 7. Relay Decision Function

The MVP relay logic should be expressed as a pure decision function.

```c
typedef enum {
    LRP_DECISION_DROP,
    LRP_DECISION_CONSUME,
    LRP_DECISION_FORWARD
} lrp_decision_t;
```

Suggested API:

```c
lrp_decision_t lrp_router_on_receive(
    lrp_router_t *router,
    const lrp_frame_t *in,
    lrp_frame_t *out
);
```

This makes relay behavior unit-testable without real radio hardware.

---

# 8. Duplicate Cache MVP

The duplicate key is:

```text
network_id + src_id + seq + type
```

Cache entry:

```c
typedef struct {
    uint16_t network_id;
    uint16_t src_id;
    uint16_t seq;
    uint8_t type;
    uint32_t timestamp_ms;
    bool used;
} lrp_dedup_entry_t;
```

MVP cache size:

```c
#define LRP_DEDUP_CACHE_SIZE 32
```

---

# 9. MVP File Layout

Recommended first implementation structure:

```text
include/
  lrp.h
  lrp_frame.h
  lrp_router.h
  lrp_dedup.h
  lrp_port.h

src/
  lrp_frame.c
  lrp_router.c
  lrp_dedup.c
  lrp_crc16.c

examples/
  simulated_three_nodes/

tests/
  test_frame.c
  test_dedup.c
  test_router.c
```

---

# 10. First Milestone

Milestone 1 is complete when these tests pass:

1. Encode and decode DATA frame.
2. Reject bad CRC.
3. Reject unsupported version.
4. Reject foreign Network ID.
5. Detect duplicate frame.
6. Relay forwards valid frame with TTL 1.
7. Relay drops valid frame with TTL 0.
8. Base station consumes frame addressed to itself.
9. Base station generates ACK.
10. Relay forwards ACK back to endpoint.

---

# 11. First Demo

The first demo should run without LoRa hardware.

Simulation:

```text
Node A creates DATA frame
Relay B receives it
Relay B forwards it
Base C receives it
Base C creates ACK
Relay B forwards ACK
Node A receives ACK
```

Only after this simulation works should the radio port be added.

---

# 12. Hardware Integration Boundary

The protocol core must not depend on a specific LoRa chip.

Required porting functions:

```c
uint32_t lrp_port_time_ms(void);
uint32_t lrp_port_random_u32(void);
bool lrp_port_radio_send(const uint8_t *data, uint16_t len);
```

Receive handling should feed raw bytes into the protocol core:

```c
lrp_on_radio_rx(data, len, rssi, snr);
```

---

# 13. MVP Completion Definition

The MVP is complete when:

1. The protocol core compiles as plain C.
2. Frame encode/decode works.
3. CRC validation works.
4. Router decision logic works.
5. Duplicate suppression works.
6. Simulated three-node relay demo works.
7. README documents how to run the simulation.

Hardware support is the next phase, not required for MVP completion.
