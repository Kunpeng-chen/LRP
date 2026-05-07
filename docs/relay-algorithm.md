# LRP Relay Algorithm

This document defines the relay and forwarding behavior of LRP.

The relay layer is the core of LRP. Its purpose is to extend LoRa coverage while avoiding uncontrolled flooding, relay loops, and excessive airtime.

The MVP relay algorithm is based on managed flooding with duplicate suppression and TTL enforcement.

---

# 1. Design Philosophy

The relay algorithm is intentionally simple.

LoRa links are:

- low bandwidth
- half duplex
- high latency
- airtime constrained
- collision sensitive

Because of these constraints, complex dynamic routing protocols are usually not appropriate for small LoRa telemetry systems.

The MVP therefore prioritizes:

1. Robustness.
2. Predictable behavior.
3. Small memory footprint.
4. Low CPU usage.
5. Simple debugging.

---

# 2. Managed Flooding

LRP uses managed flooding.

A relay forwards a packet only when all forwarding conditions are satisfied.

This differs from naive flooding because:

1. Duplicate packets are suppressed.
2. TTL limits propagation.
3. Random relay delay reduces collisions.
4. Optional overhearing suppresses redundant forwarding.

Managed flooding works well for:

- small networks
- sparse telemetry
- few relay hops
- unstable RF environments

---

# 3. Relay Processing Pipeline

When a relay receives a packet, it processes the packet in the following order:

```text
Receive Packet
    ↓
Validate Header
    ↓
Validate Length
    ↓
Validate CRC / MIC
    ↓
Check Version
    ↓
Check NetworkID
    ↓
Duplicate Cache Lookup
    ↓
Destination Check
    ↓
TTL Check
    ↓
Relay Policy Check
    ↓
Random Backoff
    ↓
Forward Packet
```

At any stage, the packet may be dropped.

---

# 4. Packet Validation

A relay must reject:

1. Unsupported protocol versions.
2. Invalid lengths.
3. Invalid CRC.
4. Invalid MIC.
5. Unknown mandatory flags.
6. Packets from foreign networks.

A relay must never forward invalid packets.

---

# 5. Duplicate Suppression

Duplicate suppression is mandatory.

Every relay stores recently seen packets in a cache.

Recommended deduplication key:

```text
NetworkID + SrcID + Seq + PacketType
```

Optional stronger dedup key:

```text
NetworkID + SrcID + DstID + Seq + PayloadHash
```

Recommended cache structure:

```text
struct DedupEntry {
    uint16_t network_id;
    uint16_t src_id;
    uint16_t seq;
    uint8_t packet_type;
    uint32_t timestamp_ms;
};
```

Recommended cache size:

| Device | Entries |
| --- | --- |
| Tiny MCU | 16–32 |
| Normal relay | 64–128 |
| Base station | 256+ |

Packets already present in cache must be dropped immediately.

---

# 6. TTL Enforcement

TTL limits relay depth.

Rules:

1. Endpoint initializes TTL.
2. Relay decrements TTL before forwarding.
3. Relay drops packet when TTL becomes zero.
4. Destination may still consume TTL=0 packets addressed to itself.

Recommended TTL values:

| Topology | TTL |
| --- | --- |
| Direct | 0 |
| One relay | 1 |
| Two relays | 2 |
| Small mesh | 2–3 |

TTL > 3 is strongly discouraged.

---

# 7. Relay Delay

A relay should not retransmit immediately.

Without randomized relay delay:

1. Multiple relays may collide.
2. Broadcast storms become likely.
3. Network stability degrades.

Recommended delay:

```text
relay_delay = base_delay + random(0, jitter)
```

Example:

```text
base_delay = 30ms
jitter = 120ms
```

Result:

```text
30–150ms forwarding delay
```

The delay should be short compared to application latency requirements.

---

# 8. Overhearing Suppression

Optional overhearing suppression reduces redundant forwarding.

Example:

```text
Relay A receives packet
Relay B receives packet
Relay A forwards first
Relay B hears Relay A forwarding same packet
Relay B cancels forwarding
```

This mechanism:

1. Reduces airtime.
2. Reduces collisions.
3. Improves scalability.

Requirements:

1. Relay keeps packet in pending-forward state.
2. Relay listens during delay window.
3. Relay cancels if same packet already forwarded.

This feature is recommended but not mandatory for MVP.

---

# 9. RSSI-Based Priority

Optional forwarding priority may be based on RSSI or SNR.

Example:

| RSSI Quality | Relay Delay |
| --- | --- |
| Excellent | Short |
| Moderate | Medium |
| Weak | Long |

This increases probability that the best-positioned relay forwards first.

Example formula:

```text
relay_delay = base + quality_penalty + random_jitter
```

This feature should remain simple.

---

# 10. ACK Handling

The MVP uses end-to-end ACK.

Flow:

```text
Endpoint → DATA
Relays forward
Base station receives
Base station → ACK
Relays forward ACK
Endpoint receives ACK
```

The ACK packet contains:

1. Source ID.
2. Destination ID.
3. Acked sequence number.
4. Optional status.

Relays treat ACK packets like normal relay packets.

---

# 11. Retry Logic

Retry logic exists only at endpoints.

Relays should not independently retry packets in MVP.

Reason:

1. Relay retries amplify congestion.
2. Multiple relay retries create storms.
3. End-to-end retry is simpler.

Endpoint retry algorithm:

```text
send packet
wait ACK timeout
if no ACK:
    random backoff
    retry
```

Recommended retry count:

```text
1–3 retries
```

---

# 12. Broadcast Handling

Broadcast packets use:

```text
DstID = 0xFFFF
```

Broadcasts are dangerous in LoRa networks.

Rules:

1. Broadcasts should rarely be used.
2. Broadcasts should usually be unconfirmed.
3. TTL for broadcasts should be minimal.
4. Relays should rate-limit broadcast forwarding.

Broadcast storms must be avoided.

---

# 13. Relay Policies

A relay may apply local policy.

Example policies:

| Policy | Purpose |
| --- | --- |
| Max packet rate | Congestion protection |
| Max TTL | Topology control |
| Allowed packet types | Security |
| Source allowlist | Access control |
| RSSI threshold | Weak packet filtering |

Policy should remain lightweight.

---

# 14. Congestion Control

LRP assumes low-duty-cycle traffic.

Congestion control recommendations:

1. Use long telemetry intervals.
2. Avoid synchronized transmissions.
3. Randomize reporting time.
4. Limit confirmed traffic.
5. Avoid large payloads.
6. Limit relay hops.

Optional future features:

- airtime estimation
- relay queue limits
- adaptive forwarding suppression

---

# 15. Security Handling

If security is enabled:

1. Relay validates MIC before forwarding.
2. Relay does not decrypt payload unless required.
3. Relay forwards encrypted payload transparently.
4. Invalid MIC packets are dropped.

This creates:

```text
store-and-forward secure relay
```

behavior.

---

# 16. Sleep and Power Model

Endpoint nodes are expected to sleep aggressively.

Relays are usually:

- mains powered
- solar powered
- large battery powered

A relay should normally remain RX-capable.

Future low-power relay mode may support:

- wake-on-radio
- scheduled receive windows
- duty-cycled listening

These are outside MVP scope.

---

# 17. Relay State Machine

Suggested relay state machine:

```text
IDLE
  ↓
RX
  ↓
VALIDATE
  ↓
DEDUP_CHECK
  ↓
TTL_CHECK
  ↓
FORWARD_PENDING
  ↓
RANDOM_DELAY
  ↓
TX_FORWARD
  ↓
IDLE
```

Drop path:

```text
INVALID → DROP
DUPLICATE → DROP
TTL_EXPIRED → DROP
POLICY_REJECT → DROP
```

---

# 18. Minimal Relay Pseudocode

```text
on_packet_receive(packet):

    if !valid(packet):
        drop(packet)
        return

    if duplicate(packet):
        drop(packet)
        return

    cache(packet)

    if packet.dst == self:
        consume(packet)
        return

    if packet.ttl == 0:
        drop(packet)
        return

    if !relay_policy(packet):
        drop(packet)
        return

    packet.ttl -= 1

    delay = random_backoff()

    wait(delay)

    if overheard_same_packet():
        cancel_forward()
        return

    radio_send(packet)
```

---

# 19. Why Managed Flooding Instead of Dynamic Routing

The MVP intentionally avoids dynamic routing.

Dynamic routing introduces:

1. Route discovery.
2. Route maintenance.
3. Route aging.
4. Route repair.
5. Topology synchronization.
6. Increased memory usage.
7. Increased airtime.

For small LoRa telemetry networks, managed flooding is often more reliable and easier to debug.

Future versions may optionally support:

- static routes
- preferred relays
- route hints
- RSSI-based next-hop learning

---

# 20. Meshtastic-Inspired Enhancements

The following features are inspired by Meshtastic-style LoRa mesh behavior and may be added later:

| Feature | Benefit |
| --- | --- |
| Passive overhearing | Reduce duplicates |
| Relay suppression | Reduce airtime |
| Adaptive forwarding | Better scaling |
| Neighbor discovery | Better topology awareness |
| Route hints | Semi-directed forwarding |
| Priority forwarding | Better relay selection |

However, LRP intentionally remains simpler than a full chat-oriented mesh network.

---

# 21. MVP Relay Requirements

A compliant MVP relay implementation must:

1. Validate packets.
2. Maintain duplicate cache.
3. Enforce TTL.
4. Apply random forwarding delay.
5. Forward eligible packets.
6. Never forward invalid packets.
7. Never forward duplicate packets.
8. Never forward TTL-expired packets.
9. Support DATA packets.
10. Support ACK packets.

Optional features may be added incrementally.
