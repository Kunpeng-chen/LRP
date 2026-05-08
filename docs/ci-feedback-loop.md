# CI Feedback Loop

Every LRP development phase must include a CI feedback loop before the phase can be considered complete.

The loop is:

```text
Implement change
    ↓
Push commit
    ↓
Fetch CI result
    ↓
If CI passes → continue
If CI fails → inspect logs → fix → push → fetch CI result again
```

---

## Required Checks

Each phase must verify:

1. CMake configure succeeds.
2. Build succeeds with warnings treated as errors.
3. `ctest --output-on-failure` succeeds.
4. All samples and simulations still build.
5. No obsolete source files are left in the active build.

---

## Failure Handling

When CI fails, do not continue to the next feature phase.

Required response:

1. Fetch the failed workflow run.
2. Fetch the failed job logs.
3. Identify the first actionable error.
4. Apply the smallest safe fix.
5. Push the fix.
6. Fetch the new CI result.
7. Repeat until CI passes or the issue is explicitly documented as blocked.

---

## Common Failure Categories

| Category | Example | Expected Fix |
| --- | --- | --- |
| Compile error | missing include | add required header |
| Warning-as-error | unused variable | remove or use variable |
| Link error | missing source file in CMake | update `CMakeLists.txt` |
| Test failure | failed assertion | fix implementation or test expectation |
| Layout error | obsolete source still compiled | remove obsolete file or CMake reference |
| CI config error | bad workflow command | fix `.github/workflows/ci.yml` |

---

## Phase Completion Rule

A phase is complete only when:

```text
latest relevant commit has passing CI
```

If CI status cannot be fetched, the phase must be marked as:

```text
implementation complete, CI verification pending
```

not as fully complete.

---

## Recommended Manual Verification

When local or CI verification is needed:

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
./build/simulated_three_nodes
./build/sample_mvp_basic_usage
```

---

## Development Discipline

Prefer small commits. If a CI failure appears, the fix should target the smallest failing layer instead of making unrelated changes.
