# Relay Networking Research Summary

## Target Scenario

The target product direction is automatic relay networking for private LoRa communication.

A relay module should support multi-hop relay networking for long-distance or complex-environment communication. Multiple independent networks should be able to run in the same physical area. In relay mode, the module should be able to work as a standalone relay after power-on, without an external control unit.

## Alignment with Current LRP Design

The current LRP design is aligned with this direction at the protocol-core level.

Implemented or planned mechanisms include:

- `network_id` for multiple coexisting networks
- `src_id`, `dst_id`, and `seq` for packet identity
- `ttl` for controlled multi-hop forwarding
- duplicate suppression to avoid repeated relay storms
- router decisions: `DROP`, `CONSUME`, `FORWARD`
- ACK path support for endpoint-to-base confirmation
- randomized relay delay to reduce collision between multiple relays
- radio / port abstraction as the boundary for future hardware integration

## Current Stage

LRP currently provides a working MVP relay protocol core:

```text
Endpoint A → Relay B → Base Station C → ACK → Relay B → Endpoint A
```

This proves the basic relay path, but the current implementation is still a protocol stack and simulation-oriented core, not yet a complete standalone relay module firmware.

## Gaps to Product Target

To fully match the target description, the following work is still required:

1. Real LoRa radio integration.
2. Standalone relay firmware mode.
3. Power-on auto-start relay behavior.
4. Persistent configuration for `network_id`, `node_id`, relay mode, and RF parameters.
5. Real multi-hop test such as `A → R1 → R2 → R3 → Base`.
6. Multi-network coexistence test in the same physical area.
7. Hardware-level validation for collision, delay, range, and packet loss.

## Conclusion

The current LRP route is correct for automatic LoRa relay networking. It already contains the key protocol primitives needed for multi-hop and multi-network relay operation. The next major step is to move from protocol-core validation to standalone radio firmware validation.
