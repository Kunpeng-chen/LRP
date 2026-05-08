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

## Required PR-Based Development Flow for Every Phase

All future phases must be developed through a feature branch and pull request.

Do not push phase implementation commits directly to `main`.

Required flow:

```text
Create feature branch
    ↓
Implement phase changes
    ↓
Push branch
    ↓
Open pull request
    ↓
Wait for PR CI / checks
    ↓
If CI passes → review and merge
If CI fails → inspect PR workflow logs → fix on same branch → wait for CI again
```

A phase is not complete unless its pull request has passing CI checks before merge.

If CI status cannot be fetched from the PR, mark the phase as:

```text
implementation complete, PR CI verification pending
```

Detailed CI rules are documented in:

```text
docs/ci-feedback-loop.md
```

---

## Required CI Feedback Loop for Every Phase

Every phase must end with a CI verification loop based on the phase pull request.

```text
Implement change on feature branch
    ↓
Push branch
    ↓
Open or update PR
    ↓
Fetch PR CI result
    ↓
If CI passes → phase can close after merge
If CI fails → inspect logs → fix → push → fetch PR CI again
```

A phase is not fully complete unless the latest PR head commit has passing CI.

Direct `main` commit status is not the source of truth for phase validation. PR checks are the source of truth.

---

## Phase 1: Stabilize the MVP Core

Goal: make the current MVP clean, maintainable, and ready for extension.

Tasks:

1. Create a feature branch for Phase 1 changes.
2. Remove obsolete single-file implementation if no longer used.
3. Ensure CMake builds only layered source files.
4. Keep all tests passing.
5. Add stack architecture documentation.
6. Keep README concise and external-user oriented.
7. Add API usage notes for each public header.
8. Open a PR and fetch PR CI result.
9. If CI fails, inspect logs, fix on the same branch, push, and re-check PR CI.

Exit criteria:

- `ctest` passes.
- `simulated_three_nodes` runs successfully.
- `sample_mvp_basic_usage` runs successfully.
- Repository layout matches the layered protocol stack design.
- Phase PR has passing CI checks before merge, or PR CI verification is explicitly pending.

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

Tasks:

1. Create a `phase-2-radio-port-abstraction` feature branch.
2. Implement port and radio abstraction.
3. Add or update tests/samples for the new boundary.
4. Open a PR.
5. Fetch PR CI result.
6. If CI fails, inspect failed job logs, fix on the same branch, push, and re-check PR CI.

Exit criteria:

- Existing simulation still works.
- A radio stub test can send bytes through encode/decode/router flow.
- No LoRa chip dependency is introduced into the protocol core.
- Phase PR has passing CI checks before merge, or PR CI verification is explicitly pending.

---

## Phase 3: Add Randomized Relay Delay

Goal: reduce collision risk when multiple relays hear the same frame.

Tasks:

1. Create a `phase-3-randomized-relay-delay` feature branch.
2. Add relay delay calculation.
3. Add configurable base delay and jitter.
4. Keep router decision pure if possible.
5. Decide whether delay belongs to router output metadata or a future scheduler layer.
6. Open a PR and fetch PR CI result.
7. If CI fails, inspect logs, fix on the same branch, push, and re-check PR CI.

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
- Phase PR has passing CI checks before merge, or PR CI verification is explicitly pending.

---

## Phase 4: ACK Retry and Endpoint Session Logic

Goal: move from single ACK demo to reusable endpoint reliability logic.

Tasks:

1. Create a `phase-4-ack-retry-session` feature branch.
2. Add endpoint send session state.
3. Track pending sequence number.
4. Match incoming ACK by `src_id`, `dst_id`, `seq`, and `type`.
5. Add retry counter.
6. Add ACK timeout handling.
7. Open a PR and fetch PR CI result.
8. If CI fails, inspect logs, fix on the same branch, push, and re-check PR CI.

Out of scope:

- relay-level retry
- dynamic routing
- congestion control

Exit criteria:

- Endpoint can resend confirmed DATA when ACK timeout occurs.
- Endpoint stops retrying after max retry count.
- Duplicate ACKs are ignored safely.
- Phase PR has passing CI checks before merge, or PR CI verification is explicitly pending.

---

## Phase 5: Real LoRa Hardware Port

Goal: run the MVP over real LoRa hardware.

Candidate hardware:

- SX1262
- SX1276
- LLCC68
- E22-style UART LoRa modules

Tasks:

1. Create a `phase-5-real-lora-port` feature branch.
2. Choose the first hardware target.
3. Implement radio send/receive binding.
4. Provide a minimal endpoint firmware example.
5. Provide a minimal relay firmware example.
6. Provide a minimal base-station firmware example.
7. Open a PR and fetch PR CI result for host-buildable code.
8. If CI fails, inspect logs, fix on the same branch, push, and re-check PR CI.

Exit criteria:

- Endpoint sends DATA over real LoRa.
- Relay forwards DATA over real LoRa.
- Base receives DATA and replies ACK.
- Endpoint receives ACK.
- Host-side PR CI passes, or PR CI verification is explicitly pending.

---

## Phase 6: Security Baseline

Goal: add minimal authenticity protection for non-test deployments.

Tasks:

1. Create a `phase-6-security-baseline` feature branch.
2. Add MIC field design.
3. Decide MIC size: 4, 8, or 16 bytes.
4. Add frame counter or anti-replay strategy.
5. Keep encryption optional.
6. Open a PR and fetch PR CI result.
7. If CI fails, inspect logs, fix on the same branch, push, and re-check PR CI.

Recommended direction:

- MVP+ uses MIC first.
- Payload encryption comes later.

Exit criteria:

- Invalid MIC packets are rejected.
- Fake ACK packets can be detected.
- Security overhead is documented.
- Phase PR has passing CI checks before merge, or PR CI verification is explicitly pending.

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

Tasks:

1. Create a feature branch per advanced feature.
2. Implement only one advanced feature per PR.
3. Open a PR and fetch PR CI result.
4. If CI fails, inspect logs, fix on the same branch, and re-check PR CI.
5. Do not start another advanced feature while CI is failing.

Exit criteria:

- Each feature is optional.
- MVP behavior remains simple and deterministic.
- Airtime impact is documented.
- Each feature PR has passing CI checks before merge, or PR CI verification is explicitly pending.

---

## Current Priority

Immediate next steps:

```text
1. Convert ongoing work to PR-based phase flow
2. Open PRs for future phase work instead of direct main commits
3. Stabilize radio / port abstraction through PR CI
4. Add randomized relay delay through PR CI
5. Add ACK retry logic through PR CI
6. Start real hardware port through PR CI
```

Do not start dynamic routing, encryption, or mesh metrics until the radio abstraction and real RF loop are working.
