# Vehicle Dynamics Monitor — 2-Week Development Roadmap

**Platform:** STM32 + MPU-6050 · HAL-based embedded firmware  
**Toolchain:** VS Code · CMake · arm-none-eabi-gcc
**Testing:** Unity · CMock · TDD (host-side)    
**Duration:** 14 days · Starting from scratch

---

## Summary

| Phase | Days | Focus |
|---|---|---|
| 1 | 1–2 | Environment & hardware setup |
| 2 | 3–4 | TDD environment & first test cases |
| 3 | 5–7 | Sensor bring-up & I2C communication |
| 4 | 8–10 | Data acquisition & signal processing |
| 5 | 11–12 | Application logic — TDD event detection |
| 6 | 13–14 | Integration, validation & documentation |

---

## Phase 1 — Environment & Hardware Setup (Days 1–2)

- [x] Install VS Code extensions: **Cortex-Debug**, **CMake Tools**, and **C/C++**
- [x] Install `arm-none-eabi-gcc`, `cmake`, `ninja-build`, and `openocd` (or ST-Link GDB server); verify all are on `PATH`
- [x] Install **STM32CubeMX** (standalone); generate HAL init code targeting my board (e.g. Nucleo-F4132ZH) with **Makefile** output, then adapt to CMake
- [ ] Configure system clock, GPIO, I2C, and UART peripherals in CubeMX and re-generate as needed
- [x] Write a `CMakeLists.txt` that compiles the CubeMX-generated sources and links the HAL/CMSIS libraries; confirm a clean `cmake --build` produces a `.elf`
- [x] Configure `launch.json` and `tasks.json` in VS Code for one-click flash and Cortex-Debug session
- [ ] Flash a blinky LED via `openocd` / ST-Link to confirm the full build-flash loop works
- [ ] Wire up MPU-6050 to STM32 (SDA, SCL, VCC, GND); connect USB-to-TTL adapter
- [x] Vendor Unity into `unity/` (copy `unity.c`, `unity.h`, `unity_internals.h` from the Unity repo)

---

## Phase 2 — TDD Environment & First Test Cases (Days 3–4)
 
- [ ] Create `tests/` directory and `tests/CMakeLists.txt`; add a `host-gcc.cmake` toolchain file for running tests on the PC
- [ ] Add `BUILD_TESTS` option to root `CMakeLists.txt`; verify `cmake -DBUILD_TESTS=ON` configures cleanly using host GCC
- [ ] Define `vehicle_dynamics.h` interface (`evaluate_dynamics()`, `VehicleEvent` enum, threshold constants) — headers first, no implementation yet
- [ ] Write `tests/unit/test_vehicle_dynamics.c` with the following failing tests (Red phase):
  - `test_no_event_when_accel_within_limits`
  - `test_harsh_braking_detected`
  - `test_rapid_acceleration_detected`
  - `test_aggressive_cornering_left`
  - `test_event_at_exact_threshold`
- [ ] Create stub `vehicle_dynamics.c` returning `EVENT_NONE` — confirm all tests compile and fail
- [ ] Implement `evaluate_dynamics()` logic to pass all tests (Green phase); refactor for clarity (Refactor phase)
- [ ] Write `tests/unit/test_mpu6050.c` with conversion tests:
  - `test_raw_bytes_reconstruct_to_signed_int`
  - `test_raw_to_gforce_at_4g_range`
  - `test_negative_raw_value_gives_negative_gforce`
- [ ] Implement `reconstruct_raw_value()` and `raw_to_gforce()` to pass conversion tests
---
 
## Phase 3 — Sensor Bring-Up & I2C Communication (Days 5–7)
 
- [ ] Write HAL I2C read/write helper functions for register-level access
- [ ] Wake MPU-6050 from sleep via `PWR_MGMT_1` register write
- [ ] Verify device identity by reading `WHO_AM_I` register (expect `0x68`)
- [ ] Configure sensor range (e.g. ±4g) and sample rate via `ACCEL_CONFIG`
- [ ] Read raw 6-byte acceleration burst; print raw hex over UART to confirm comms
---
 
## Phase 4 — Data Acquisition & Signal Processing (Days 8–10)
 
- [ ] Reconstruct signed 16-bit values from two raw bytes per axis
- [ ] Convert raw counts to g-force using sensitivity scale factor (e.g. 8192 LSB/g for ±4g)
- [ ] Implement fixed-interval polling loop using `HAL_GetTick()` (non-blocking)
- [ ] Output formatted acceleration values (X, Y, Z) over UART in human-readable form
- [ ] Validate readings by tilting sensor; confirm gravity appears on the correct axis (~1.0 g)
---
 
## Phase 5 — Application Logic: TDD Event Detection (Days 11–12)
 
- [ ] Review and extend `test_vehicle_dynamics.c` with any edge cases uncovered during sensor work (e.g. simultaneous lateral + longitudinal events)
- [ ] Implement debounce logic using a minimum hold-count; write a test for it before implementing (`test_event_requires_hold_count`)
- [ ] Wire `evaluate_dynamics()` into the main acquisition loop — inputs come from `raw_to_gforce()` output
- [ ] Classify detected events and transmit human-readable UART messages via `uart_logger.c`
- [ ] Run full test suite on host (`cmake --build build-test && ./build-test/tests/test_vehicle_dynamics`); all tests must pass before flashing
---
 
## Phase 6 — Integration, Validation & Documentation (Days 13–14)
 
- [ ] End-to-end test on hardware: tilt/shake sensor and confirm correct event messages appear on serial terminal
- [ ] Confirm all unit tests still pass after integration (`cmake -DBUILD_TESTS=ON` clean build)
- [ ] Review code for modularity: sensor HAL layer fully separated from application logic layer
- [ ] Add header comments and inline documentation throughout source files
- [ ] Update README: hardware wiring diagram, CMake build commands, test commands, flash instructions, UART baud rate, test procedure
- [ ] Record known limitations and draft future work backlog (filtering, RTOS, CAN output…)
---
 
## Notes
 
- **TDD boundary:** `vehicle_dynamics.c` must contain zero HAL calls — pure float in, event enum out. This is what makes it testable on the host without mocking the entire HAL.
- **Red → Green → Refactor:** Write a failing test first, implement the minimum code to pass it, then clean up. Never write implementation without a failing test in front of it.
- **Host vs target builds:** Always keep two CMake configurations — `build/` (ARM, for flashing) and `build-test/` (host GCC, for running tests). Never cross them.
- **CMake + CubeMX:** CubeMX generates a Makefile project by default — you'll need to manually write (or adapt) a `CMakeLists.txt` that pulls in the same source lists and compiler flags. Budget extra time in Phase 1 for this; getting it right once means clean incremental builds for the rest of the project.
- **Phase 3 buffer:** I2C bring-up on a physical MPU-6050 can surface wiring or pull-up resistor issues — budget an extra half-day if needed.
- **Gravity validation:** Before writing any threshold logic, confirm the sensor returns ~1.0 g on the correct axis when tilted. This is your functional proof of a working signal chain.
- **Modular architecture:** Keep the sensor HAL layer separate from application logic throughout. This is what makes the component extensible for future RTOS, CAN, or filtering work.
- **Testing without a vehicle:** Exploit gravity-based tilting to simulate longitudinal and lateral acceleration during development.
---
 
## Future Work (Post-Roadmap)
 
- Introduce digital filtering (moving average or IIR) to reduce noise
- Replace blocking delays with non-blocking, time-based scheduling
- Add sensor calibration and bias compensation
- Abstract motion detection logic into a reusable service layer
- Integrate RTOS support for task-based execution
- Support additional outputs (CAN, logging, or structured diagnostics)
- Extend detection logic to support scoring or severity classification

# Vehicle Dynamics Monitor — Project Draft Folder Structure

```
vehicle-dynamics-monitor/
│
├── .vscode/
│   ├── launch.json                       ← flash + Cortex-Debug session config
│   └── tasks.json                        ← CMake build tasks
│
├── cmake/
│   ├── arm-none-eabi.cmake               ← cross-compile toolchain (STM32 target)
│   └── host-gcc.cmake                    ← host toolchain (for running tests on PC)
│
├── Core/
│   ├── Inc/
│   │   ├── main.h
│   │   ├── mpu6050.h                     ← sensor driver interface
│   │   ├── vehicle_dynamics.h            ← event detection interface (HAL-free)
│   │   └── uart_logger.h                 ← diagnostics interface
│   └── Src/
│       ├── main.c
│       ├── mpu6050.c                     ← sensor driver (HAL-dependent)
│       ├── vehicle_dynamics.c            ← pure logic (HAL-free, host-testable)
│       └── uart_logger.c                 ← UART output (HAL-dependent)
│
├── Drivers/
│   └── STM32F4xx_HAL_Driver/             ← CubeMX-generated HAL & CMSIS
│
├── tests/
│   ├── CMakeLists.txt                    ← test build config (host GCC)
│   ├── mocks/
│   │   ├── mock_mpu6050.h                ← CMock-generated mock for sensor driver
│   │   └── mock_uart_logger.h            ← CMock-generated mock for UART
│   ├── unit/
│   │   ├── test_vehicle_dynamics.c       ← event detection logic tests
│   │   └── test_mpu6050.c               ← sensor data conversion tests
│   └── integration/
│       └── test_acquisition_pipeline.c  ← end-to-end acquisition flow tests
│
├── unity/
│   ├── unity.c                           ← Unity test framework (vendored)
│   ├── unity.h
│   └── unity_internals.h
│
├── CMakeLists.txt                        ← root build config (firmware + test toggle)
├── .gitignore
└── README.md
```

## Notes

- `vehicle_dynamics.c` contains **zero HAL calls** — pure logic only, making it fully testable on the host without mocking the HAL
- `build/` and `build-test/` are generated at runtime and excluded by `.gitignore`
- Two separate CMake configurations:
  - `cmake -B build` → ARM target (firmware, flash to STM32)
  - `cmake -B build-test -DBUILD_TESTS=ON` → host GCC (unit tests, run on PC)