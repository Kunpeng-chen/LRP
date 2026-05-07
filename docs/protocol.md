# LRP Protocol Specification

LRP, LoRa Relay Protocol, is a lightweight relay protocol designed for private LoRa networks. It is intended for low-rate, small-payload, long-range telemetry systems where a direct LoRa link between an endpoint and a base station is unreliable or unavailable.

LRP is not LoRaWAN. It does not implement LoRaWAN MAC, join procedure, network server integration, LoRaWAN security, or Class A/B/C receive windows. LRP is designed for private LoRa P2P or LoRa DTU style systems where the application owns both the endpoint firmware and the relay/base-station firmware.

## 1. Design Goals

LRP has the following goals:

1. Extend LoRa P2P coverage with one or more relay nodes.
2. Keep the protocol small enough for resource-constrained MCUs.
3. Support managed flooding as the default relay mode.
4. Prevent relay loops using TTL and duplicate suppression.
5. Provide optional ACK-based reliability for important data.
6. Allow future support for learned routes or static routes.
7. Keep the radio payload short to reduce airtime.
8. Separate protocol logic from the radio driver.

## 2. Non-goals

LRP does not try to provide:

1. LoRaWAN compatibility.
2. IP networking.
3. High-throughput transmission.
4. Real-time deterministic delivery.
5. Unlimited multi-hop mesh routing.
6. Automatic large-scale network management.
7. Transparent byte-stream forwarding without packet boundaries.

## 3. Target Scenarios

Typical LRP deployments include:

- Remote sensors that cannot directly reach the base station.
- Agricultural monitoring networks.
- Reservoir, fish pond, greenhouse, mountain, or field telemetry.
- Industrial low-rate RS485/MCU telemetry over private LoRa.
- Battery-powered sensor nodes with mains-powered or solar-powered relays.

LRP is best suited for small messages, low sending frequency, and a limited number of relay hops.

## 4. Network Roles

LRP defines three logical roles.

### 4.1 Endpoint

An endpoint produces application data. It usually sleeps most of the time, wakes up, sends one or more packets, optionally waits for ACK, and then returns to sleep.

### 4.2 Relay

A relay receives packets and forwards eligible packets according to TTL, duplicate cache, and forwarding policy. A relay may also produce its own telemetry, such as battery voltage, RSSI statistics, or health status.

### 4.3 Base Station

A base station is the sink for endpoint data. It may connect to a host, PLC, computer, gateway, or application server through UART, USB, Ethernet, Wi-Fi, cellular, or another local interface.

A base station may also send downlink commands to endpoints, but downlink must be used sparingly because LoRa is half-duplex and airtime is limited.

## 5. Addressing Model

Every LRP node has a Node ID.

Recommended Node ID size:

- MVP: 16-bit Node ID.
- Extended mode: 32-bit Node ID.

Reserved addresses:

| Address | Meaning |
| --- | --- |
| `0x0000` | Invalid or unassigned node |
| `0x0001` | Default base station |
| `0xFFFE` | Local test address |
| `0xFFFF` | Broadcast |

For most MVP deployments, 16-bit addressing is enough. A network with multiple independent LRP deployments should also use a Network ID to reduce cross-network packet acceptance.

## 6. Packet Classes

LRP defines the following packet classes:

| Class | Description |
| --- | --- |
| DATA | Application payload from source to destination |
| ACK | Acknowledgement for a received DATA packet |
| HELLO | Node announcement and capability advertisement |
| ROUTE_HINT | Optional route information for future route learning |
| CONTROL | Configuration, status request, or command |
| DIAG | Diagnostics, statistics, or test packet |

The MVP only requires DATA and ACK. HELLO is strongly recommended for debugging and network discovery.

## 7. Forwarding Model

The default forwarding model is managed flooding.

When a relay receives a packet, it forwards the packet only when all conditions are true:

1. The packet is valid.
2. The packet has not been seen before.
3. The packet destination is not the relay itself.
4. TTL is greater than zero.
5. The relay policy allows forwarding this packet type.
6. The relay is not rate-limited.

Before forwarding, the relay decrements TTL by one and records itself as the previous hop if the selected frame format includes a previous-hop field.

Managed flooding is simpler and more robust than dynamic routing in small LoRa networks. It works well when the number of nodes is limited and traffic rate is low.

## 8. Duplicate Suppression

Every node keeps a recent-packet cache. A packet is considered duplicate when the tuple below has been seen recently:

```text
NetworkID + SrcID + Seq + PacketType
```

For stronger duplicate detection, the node may include DstID and payload hash:

```text
NetworkID + SrcID + DstID + Seq + PacketType + PayloadHash
```

Recommended cache size:

- Tiny MCU: 16 to 32 entries.
- Normal MCU: 64 to 128 entries.
- Base station: 256 or more entries.

Recommended cache lifetime:

- 2 to 5 times the expected maximum end-to-end retry window.
- Usually 30 to 300 seconds depending on reporting interval.

## 9. Reliability Model

LRP supports three delivery modes:

| Mode | Description | Use Case |
| --- | --- | --- |
| Unconfirmed DATA | No ACK expected | Periodic telemetry, non-critical data |
| Confirmed DATA | End-to-end ACK expected | Alarms, control results, important measurements |
| Local ACK | Next-hop ACK, optional | Advanced mode, not required by MVP |

The MVP should implement end-to-end ACK only. A base station sends an ACK back to the source when a confirmed DATA packet reaches the destination.

The source retries when no ACK is received within the ACK timeout. Retry count should be small because repeated LoRa transmissions increase collision probability and airtime.

Recommended defaults:

| Parameter | Value |
| --- | --- |
| Max retries | 1 to 3 |
| ACK timeout | Based on airtime and hop count, usually 2 to 10 seconds |
| Random backoff | Required before retry |
| Confirmed traffic ratio | Low, preferably only alarms or commands |

## 10. TTL and Hop Count

TTL prevents infinite forwarding loops.

Recommended TTL values:

| Scenario | TTL |
| --- | --- |
| Direct only | 0 |
| One relay | 1 |
| Two relays | 2 |
| Small field network | 2 to 3 |

TTL higher than 3 should be avoided unless the network is carefully planned. Each additional hop consumes extra airtime and increases collision risk.

## 11. Timing and Backoff

A relay should not forward immediately after receiving a valid packet. It should wait for a randomized backoff delay to reduce simultaneous relay collisions.

Recommended relay backoff:

```text
relay_delay = base_delay + random(0, jitter_window)
```

The delay may be adjusted by RSSI, SNR, hop count, or relay priority.

Example policy:

- Relay with better RSSI forwards earlier.
- Relay with weaker RSSI forwards later.
- Low-priority relay may suppress forwarding if it hears another relay forwarding the same packet first.

## 12. Security Model

LRP should include at least message authentication for non-test deployments.

Recommended security levels:

| Level | Description |
| --- | --- |
| L0 | No security, test only |
| L1 | CRC only, test or lab only |
| L2 | AES-CTR encryption + MIC |
| L3 | AEAD such as AES-CCM or ChaCha20-Poly1305 |

Recommended production baseline:

- Per-network root key or per-device key.
- Frame counter or monotonic sequence protection.
- MIC over header and payload.
- Reject packets with invalid MIC.
- Reject stale frame counters when security is enabled.

Because LoRa payloads are small, security overhead must be considered carefully. For the MVP, a 4-byte MIC may be acceptable. For stronger deployments, use 8 or 16 bytes.

## 13. Airtime Constraints

LRP must minimize airtime.

Guidelines:

1. Keep application payload small, ideally below 50 bytes.
2. Avoid frequent confirmed messages.
3. Limit TTL.
4. Use binary encoding instead of JSON.
5. Prefer event-driven or periodic telemetry with long intervals.
6. Avoid multi-hop broadcast storms.

## 14. Recommended MVP

The first implementation should include:

1. 16-bit Network ID.
2. 16-bit Node ID.
3. DATA packet.
4. ACK packet.
5. Sequence number.
6. TTL.
7. Duplicate cache.
8. Random relay backoff.
9. CRC16.
10. Optional 4-byte MIC placeholder.

The MVP may omit:

1. Dynamic routing.
2. Fragmentation.
3. Time synchronization.
4. Local ACK.
5. OTA configuration.
6. Large payload support.

## 15. Versioning

LRP packets include a protocol version field. Incompatible changes must increment the major version. Compatible additions should use flags or TLV extensions.

Recommended initial version:

```text
LRP_VERSION = 0x01
```

Nodes should drop packets with unsupported major versions.

## 16. Interoperability Contract

A valid LRP implementation must:

1. Parse the fixed header.
2. Validate length and CRC.
3. Validate version.
4. Check duplicate cache before forwarding.
5. Enforce TTL.
6. Decrement TTL before relay forwarding.
7. Preserve SrcID, DstID, Seq, and PacketType during forwarding.
8. Generate ACK for confirmed DATA addressed to itself.
9. Never forward invalid packets.
10. Apply random backoff before forwarding.

## 17. Implementation Boundaries

The protocol core should not directly depend on a specific LoRa chip. Radio operations should be hidden behind a porting interface:

```text
lrp_radio_send(buffer, length, radio_params)
lrp_radio_receive(buffer, max_length, metadata)
lrp_get_time_ms()
lrp_random_u32()
```

This allows the same LRP core to run on SX1262, SX1276, LLCC68, E22 modules, RAK modules, or simulated test backends.
