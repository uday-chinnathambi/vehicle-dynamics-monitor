# Vehicle Dynamics Monitor — 2-Week Development Roadmap

**Platform:** STM32 + MPU-6050 · HAL-based embedded firmware  
**Toolchain:** VS Code · CMake · arm-none-eabi-gcc  
**Duration:** 14 days · Starting from scratch

---

## Summary

| Phase | Days | Focus |
|---|---|---|
| 1 | 1–2 | Environment & hardware setup |
| 2 | 3–5 | Sensor bring-up & I2C communication |
| 3 | 6–8 | Data acquisition & signal processing |
| 4 | 9–11 | Application logic — event detection |
| 5 | 12–14 | Integration, validation & documentation |

---

## Phase 1 — Environment & Hardware Setup (Days 1–2)

- [ ] Install VS Code extensions: **Cortex-Debug**, **CMake Tools**, and **C/C++**
- [ ] Install `arm-none-eabi-gcc`, `cmake`, `ninja-build`, and `openocd` (or ST-Link GDB server); verify all are on `PATH`
- [ ] Install **STM32CubeMX** (standalone); generate HAL init code targeting your board (e.g. Nucleo-F446RE) with **Makefile** output, then adapt to CMake
- [ ] Configure system clock, GPIO, I2C, and UART peripherals in CubeMX and re-generate as needed
- [ ] Write a `CMakeLists.txt` that compiles the CubeMX-generated sources and links the HAL/CMSIS libraries; confirm a clean `cmake --build` produces a `.elf`
- [ ] Configure `launch.json` and `tasks.json` in VS Code for one-click flash and Cortex-Debug session
- [ ] Flash a blinky LED via `openocd` / ST-Link to confirm the full build-flash loop works
- [ ] Wire up MPU-6050 to STM32 (SDA, SCL, VCC, GND); connect USB-to-TTL adapter

---

## Phase 2 — Sensor Bring-Up & I2C Communication (Days 3–5)

- [ ] Write HAL I2C read/write helper functions for register-level access
- [ ] Wake MPU-6050 from sleep via `PWR_MGMT_1` register write
- [ ] Verify device identity by reading `WHO_AM_I` register (expect `0x68`)
- [ ] Configure sensor range (e.g. ±4g) and sample rate via `ACCEL_CONFIG`
- [ ] Read raw 6-byte acceleration burst; print raw hex over UART to confirm comms

---

## Phase 3 — Data Acquisition & Signal Processing (Days 6–8)

- [ ] Reconstruct signed 16-bit values from two raw bytes per axis
- [ ] Convert raw counts to g-force using sensitivity scale factor (e.g. 8192 LSB/g for ±4g)
- [ ] Implement fixed-interval polling loop using `HAL_GetTick()` (non-blocking)
- [ ] Output formatted acceleration values (X, Y, Z) over UART in human-readable form
- [ ] Validate readings by tilting sensor; confirm gravity appears on the correct axis (~1.0 g)

---

## Phase 4 — Application Logic: Event Detection (Days 9–11)

- [ ] Define threshold constants for harsh braking, rapid acceleration, and aggressive cornering
- [ ] Implement longitudinal event detection (X-axis vs. braking/accel thresholds)
- [ ] Implement lateral event detection (Y-axis vs. cornering threshold)
- [ ] Classify detected events and transmit human-readable UART messages
- [ ] Add event debounce / minimum hold-time to suppress noise-driven false triggers

---

## Phase 5 — Integration, Validation & Documentation (Days 12–14)

- [ ] End-to-end test: tilt/shake sensor and confirm correct event messages appear on serial terminal
- [ ] Review code for modularity: separate sensor HAL layer from application logic layer
- [ ] Add header comments and inline documentation throughout source files
- [ ] Update README: hardware wiring diagram, CMake build commands, flash instructions, UART baud rate, test procedure
- [ ] Record known limitations and draft future work backlog (filtering, RTOS, CAN output…)

---

## Notes

- **CMake + CubeMX:** CubeMX generates a Makefile project by default — you'll need to manually write (or adapt) a `CMakeLists.txt` that pulls in the same source lists and compiler flags. Budget extra time in Phase 1 for this; getting it right once means clean incremental builds for the rest of the project.
- **Phase 2 buffer:** I2C bring-up on a physical MPU-6050 can surface wiring or pull-up resistor issues — budget an extra half-day if needed.
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