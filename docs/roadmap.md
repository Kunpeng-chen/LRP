# LRP Roadmap

This document defines the development plan after the MVP relay core.

The MVP currently provides:

- Frame layer
- Dedup layer
- Router decision layer
- Three-node relay simulation
- Basic usage sample
- CMake tests
- GitHub Actions CI

The next goal is to move from an in-memory protocol core to a hardware-ready LoRa relay stack.

---

## Required CI Feedback Loop for Every Phase

Every phase must end with a CI verification loop.

```text
Implement change
    ↓
Push commit
    ↓
Fetch CI result
    ↓
If CI passes → phase can close
If CI fails → inspect logs → fix → push → fetch CI again
```

A phase is not fully complete unless the latest relevant commit has passing CI.

If CI status cannot be fetched, mark the phase as:

```text
implementation complete, CI verification pending
```

Detailed rules are documented in:

```text
docs/ci-feedback-loop.md
```

---

## Phase 1: Stabilize the MVP Core

Goal: make the current MVP clean, maintainable, and ready for extension.

Tasks:

1. Remove obsolete single-file implementation if no longer used.
2. Ensure CMake builds only layered source files.
3. Keep all tests passing.
4. Add stack architecture documentation.
5. Keep README concise and external-user oriented.
6. Add API usage notes for each public header.
7. Fetch CI result for the latest phase commit.
8. If CI fails, inspect logs, fix, push, and re-check CI.

Exit criteria:

- `ctest` passes.
- `simulated_three_nodes` runs successfully.
- `sample_mvp_basic_usage` runs successfully.
- Repository layout matches the layered protocol stack design.
- Latest phase commit has passing CI, or CI verification is explicitly pending.

---

## Phase 2: Add Radio / Port Abstraction

Goal: introduce a hardware-independent radio boundary.

Planned files:

```text
include/lrp_port.h
include/lrp_radio.h
src/lrp_port_stub.c
src/lrp_radio.c
```

Responsibilities:

- abstract time source
- abstract random number source
- abstract radio send
- abstract radio receive callback boundary
- keep LRP core independent from SX1262/SX1276/LLCC68 drivers

Suggested API:

```c
uint32_t lrp_port_time_ms(void);
uint32_t lrp_port_random_u32(void);
bool lrp_radio_send(const uint8_t *data, uint16_t len);
void lrp_on_radio_rx(const uint8_t *data, uint16_t len, int16_t rssi, int8_t snr);
```

CI feedback steps:

1. Fetch CI result after port/radio abstraction commits.
2. If build fails, inspect failed job logs.
3. Fix compile, warning, link, or test failures before continuing.
4. Re-check CI after the fix commit.

Exit criteria:

- Existing simulation still works.
- A radio stub test can send bytes through encode/decode/router flow.
- No LoRa chip dependency is introduced into the protocol core.
- Latest phase commit has passing CI, or CI verification is explicitly pending.

---

## Phase 3: Add Randomized Relay Delay

Goal: reduce collision risk when multiple relays hear the same frame.

Tasks:

1. Add relay delay calculation.
2. Add configurable base delay and jitter.
3. Keep router decision pure if possible.
4. Decide whether delay belongs to router output metadata or a future scheduler layer.
5. Fetch CI result after implementation.
6. If CI fails, inspect logs, fix, push, and re-check CI.

Possible design:

```c
typedef struct {
    lrp_decision_t decision;
    uint32_t delay_ms;
} lrp_router_result_t;
```

Exit criteria:

- Relay forwarding can return a randomized delay.
- Tests verify delay range.
- Current simple router API remains backward compatible or is intentionally upgraded.
- Latest phase commit has passing CI, or CI verification is explicitly pending.

---

## Phase 4: ACK Retry and Endpoint Session Logic

Goal: move from single ACK demo to reusable endpoint reliability logic.

Tasks:

1. Add endpoint send session state.
2. Track pending sequence number.
3. Match incoming ACK by `src_id`, `dst_id`, `seq`, and `type`.
4. Add retry counter.
5. Add ACK timeout handling.
6. Fetch CI result after implementation.
7. If CI fails, inspect logs, fix, push, and re-check CI.

Out of scope:

- relay-level retry
- dynamic routing
- congestion control

Exit criteria:

- Endpoint can resend confirmed DATA when ACK timeout occurs.
- Endpoint stops retrying after max retry count.
- Duplicate ACKs are ignored safely.
- Latest phase commit has passing CI, or CI verification is explicitly pending.

---

## Phase 5: Real LoRa Hardware Port

Goal: run the MVP over real LoRa hardware.

Candidate hardware:

- SX1262
- SX1276
- LLCC68
- E22-style UART LoRa modules

Tasks:

1. Choose the first hardware target.
2. Implement radio send/receive binding.
3. Provide a minimal endpoint firmware example.
4. Provide a minimal relay firmware example.
5. Provide a minimal base-station firmware example.
6. Fetch CI result for host-buildable code.
7. If CI fails, inspect logs, fix, push, and re-check CI.

Exit criteria:

- Endpoint sends DATA over real LoRa.
- Relay forwards DATA over real LoRa.
- Base receives DATA and replies ACK.
- Endpoint receives ACK.
- Host-side CI still passes, or CI verification is explicitly pending.

---

## Phase 6: Security Baseline

Goal: add minimal authenticity protection for non-test deployments.

Tasks:

1. Add MIC field design.
2. Decide MIC size: 4, 8, or 16 bytes.
3. Add frame counter or anti-replay strategy.
4. Keep encryption optional.
5. Fetch CI result after implementation.
6. If CI fails, inspect logs, fix, push, and re-check CI.

Recommended direction:

- MVP+ uses MIC first.
- Payload encryption comes later.

Exit criteria:

- Invalid MIC packets are rejected.
- Fake ACK packets can be detected.
- Security overhead is documented.
- Latest phase commit has passing CI, or CI verification is explicitly pending.

---

## Phase 7: Optional Advanced Relay Features

Goal: improve scaling without becoming a full mesh network.

Possible features:

- overhearing suppression
- preferred relay ID
- RSSI/SNR-based forwarding priority
- route hints
- relay health beacons
- broadcast rate limiting

CI feedback steps:

1. Fetch CI result after each feature.
2. If CI fails, inspect logs, fix, push, and re-check CI.
3. Do not start another advanced feature while CI is failing.

Exit criteria:

- Each feature is optional.
- MVP behavior remains simple and deterministic.
- Airtime impact is documented.
- Latest feature commit has passing CI, or CI verification is explicitly pending.

---

## Current Priority

Immediate next steps:

```text
1. Stabilize layered stack cleanup
2. Add radio / port abstraction
3. Fetch CI result and fix any failure
4. Add randomized relay delay
5. Add ACK retry logic
6. Start real hardware port
```

Do not start dynamic routing, encryption, or mesh metrics until the radio abstraction and real RF loop are working.
