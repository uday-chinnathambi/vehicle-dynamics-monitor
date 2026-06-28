# Topic

## Cmake
Configure Cmake from VS code =>  ctrl + shift + p => Cmake:Configure => Select tool kit => In this case, arm-none-eabi-gcc.
Now, VS code cmake extention set the compiler and reads the available cmake configuration from cmake directory.

Using "Run Task" select the job you would like to perform. Here I did run cmake:build.

CMake is a build system generator. It reads your CMakeLists.txt and produces the actual build instructions, but it doesn't do the compiling itself. Think of it as a translator — you describe what to build (sources, flags, linker scripts, toolchain), and CMake figures out the concrete commands needed.

Ninja is a build system — it actually executes those commands. It calls arm-none-eabi-gcc, runs the linker, and produces your .elf. It's designed to be fast and minimal, with no logic of its own — it just runs a pre-generated list of commands as efficiently as possible.

The pipeline in your project is:
CMakeLists.txt
     │
     ▼
cmake -G Ninja ...        ← "generate Ninja build files"
     │
     ▼
build.ninja               ← generated, you never edit this
     │
     ▼
cmake --build ./build     ← CMake calls Ninja under the hood
     │
     ▼
your_project.elf

### Why not just use Make? 
You could — CMake can generate Makefiles too (it's the default). Ninja is preferred in embedded projects because it's significantly faster on incremental builds and handles parallel compilation better. On a small firmware project the difference is minor, but it's the modern standard and what most STM32 CMake templates default to.

### Why not call Ninja directly? 
You can (ninja -C build), and it works fine. But using cmake --build ./build is more portable — it works regardless of whether the backend is Ninja, Make, or something else, which matters if someone else clones your project on a different setup.

So in short: CMake owns the logic, Ninja owns the speed. CMake writes the recipe, Ninja cooks it.

## Debuggers

### Introduction
- A debugger pauses a running program and lets you inspect registers, memory and variables without rebuilding.
- A debug probe is the hardware bridge between your laptop and the microcontroller (ST-Link, J-Link).
- SWD and JTAG are the debug interfaces on the microcontroller side. SWD is the standard choice for ARM Cortex-M.
- GDB issues debug commands. OpenOCD bridges GDB to the debug probe.
- Compile with -g to include debug symbols in the .elf file.

### OpenOCD vs ST-Link GDB Server

| | OpenOCD | ST-Link GDB Server |
|---|---|---|
| **Type** | Open source debug server | Proprietary debug server by ST |
| **Probe support** | ST-Link, J-Link, CMSIS-DAP, and more | ST-Link only |
| **Target support** | Many vendors (STM32, NXP, TI, RISC-V…) | STM32 / STM8 only |
| **VS Code integration** | Via Cortex-Debug (`servertype: openocd`) | Via Cortex-Debug (`servertype: stlink`) |
| **Config files** | Requires `.cfg` files (interface + target) | Auto-detects target, minimal config |
| **Setup effort** | Slightly more (need correct `.cfg` files) | Simpler out of the box |
| **GDB port** | 3333 (default) | 61234 (default) |
| **Semihosting** | Supported | Supported |
| **ITM / SWO tracing** | Supported | Supported |
| **RTOS thread awareness** | Supported (FreeRTOS, ThreadX…) | Limited |
| **Platform** | Linux, macOS, Windows | Windows primary; limited on Linux/macOS |
| **Standalone install** | Yes | Bundled with STM32CubeProgrammer |

---

### Bottom Line

**OpenOCD** is the better fit for a VS Code + CMake workflow — it is platform-independent,
integrates cleanly with Cortex-Debug, and is not tied to ST's tooling.

**ST-Link GDB Server** is worth knowing about if you use STM32CubeProgrammer or need
faster flash speeds on larger binaries, but for a firmware component project OpenOCD covers
everything you need.

## Peripherals

### USART vs UART

UART stands for Universal Asynchronous Receiver Transmitter. The key word is asynchronous — the two devices communicating don't share a clock signal. Instead they agree in advance on a speed (baud rate), and each side uses its own internal clock to time the bits.

USART stands for Universal Synchronous Asynchronous Receiver Transmitter. It's a superset of UART — it does everything UART does, but it can also operate in synchronous mode, where a shared clock line is added to the connection. 

| | UART | USART |
|---|---|---|
| **Clock line** | None — each side uses its own | Optional shared clock line |
| **Wires needed** | 2 (TX, RX) | 2 async, or 3 sync (TX, RX, CK) |
| **Speed** | Up to ~5 Mbps typically | Higher, due to clock synchronisation |
| **Complexity** | Simple | Slightly more config |
| **Typical use** | Debug output, GPS, Bluetooth modules, PC communication | Higher-speed peripherals, SPI-like device comms |

**When to use each**

*Async UART* — debug messages, GPS modules, Bluetooth, ESP32, PC communication, USB-to-TTL adapters

*USART synchronous mode* — higher throughput, clock-sensitive peripherals, noisy environments, SPI alternative

**ST-USAGE** : There were ten UART channels on this micro but I am going to use the one that was already configured and linked via ST-Link for simplicity. If needed additional UART to USB like FT232 can be used.
| Pin name | Function | Virtual COM port |
|---|---|---|
| PD8 | USART3 TX | via ST-LINK |
| PD9 | USART3 RX | via ST-LINK |

### Inter-integrated circuit interface (I2C)

### Issues faced - learining

Identified that cmake\stm32cubemx\CMakeLists.txt did not contain I2C and uart related paths in drivers folder.
IOC file configuration was wrong with GPIOS conntected to LEDs were defined as inputs instead of output.

### CMakePresets.json

Standardizes build configurations so developers, CI, and IDEs use identical flags without manual `-D` flag passing.

#### Presets

| Preset | Purpose |
|---|---|
| `default` (hidden) | Base: Ninja generator, ARM cross-compiler toolchain, `build/<presetName>` output |
| `Debug` | Inherits default + `CMAKE_BUILD_TYPE=Debug` |
| `Release` | Inherits default + `CMAKE_BUILD_TYPE=Release` |
| `RelWithDebInfo` | Inherits default + `CMAKE_BUILD_TYPE=RelWithDebInfo` |
| `MinSizeRel` | Inherits default + `CMAKE_BUILD_TYPE=MinSizeRel` |
| `test-debug` | Host build (no ARM toolchain), output to `build-test/`, `BUILD_TESTS=ON` |

### Usage

```bash
cmake --preset Debug         # configure
cmake --build --preset Debug # build
```

---

### TDD — vehicle_dynamics module (2026-06-21)

#### What was done

Implemented the `evaluate_dynamics()` function following the Red → Green → Refactor TDD cycle.

- Created `Core/Inc/vehicle_dynamics.h` — interface first, no implementation
- Wrote 7 failing unit tests in `tests/unit/test_vehicle_dynamics.c`
- Fixed the build so tests could compile and link
- Implemented the logic to pass all 7 tests


#### Key takeaways

- Always verify that the source under test is compiled into the test executable — a missing source gives a linker error, not a compiler error, which is harder to trace.
- Use `nm <object_file> | grep " U "` to list undefined symbols and pinpoint linker failures quickly.
- G-force values must be `float` — `uint32_t` silently truncates fractional values and cannot represent negatives.

---

## MPU-6050 Sensitivity — Where 8192 LSB/g comes from (2026-06-22)

The MPU-6050 outputs a signed 16-bit integer per axis (`−32768` to `+32767`). That range is mapped across the configured full-scale range. For ±4g:

```
Sensitivity = 32768 counts ÷ 4g = 8192 LSB/g
```

So `1g = 8192 raw counts` and `g-force = raw_value / 8192.0f`.

### All four ranges (from the datasheet)

| `AFS_SEL` | Full-scale | Sensitivity |
|---|---|---|
| 0 | ±2g | 16384 LSB/g |
| 1 | ±4g | **8192 LSB/g** ← project uses this |
| 2 | ±8g | 4096 LSB/g |
| 3 | ±16g | 2048 LSB/g |

Each time the range doubles, sensitivity halves — same 16-bit output, wider physical range, coarser resolution.

### Why ±4g for this project

Typical vehicle events fall between 0.3g–0.8g. ±2g covers this but leaves no headroom. ±4g provides comfortable margin while keeping enough resolution to distinguish 0.4g from 0.5g (~820 count difference).

---

## Phase 3 — I2C Bring-Up & Debugging (2026-06-27)

### What was done

- Added HAL I2C helper functions to `mpu6050.h/c`: `mpu6050_read_register`, `mpu6050_write_register`, `mpu6050_wake`, `mpu6050_verify_identity`
- Called wake and identity check from `main.c` with UART output to confirm result
- Debugged "wake FAILED / WHO_AM_I FAILED" on serial terminal
- Fixed a CMake/preprocessor gap that broke the host test build

---

### Issues encountered and how they were resolved

#### 1. HAL MSP — UART works without configuring GPIOD in `MX_GPIO_Init`

UART and I2C GPIO pins are NOT configured in `MX_GPIO_Init`. CubeMX places peripheral-owned GPIO setup inside `HAL_UART_MspInit` / `HAL_I2C_MspInit` callbacks in `stm32f4xx_hal_msp.c`. These are called automatically from inside `HAL_UART_Init` / `HAL_I2C_Init`. `MX_GPIO_Init` is only for general-purpose pins (LEDs, buttons).

---

#### 2. MPU-6050 reporting "wake FAILED / WHO_AM_I FAILED"

Two root causes identified:

**Hardware — AD0 pin not connected to GND**
The MPU-6050 I2C address is determined by the AD0 pin:
- AD0 = GND → address `0x68` (what the code uses)
- AD0 = floating or HIGH → address `0x69` → every transaction NACKs

Fix: wire AD0 pin to GND on the MPU-6050 module.

**Hardware — Pull-up resistors on SDA/SCL**
I2C is open-drain. The MCU can only pull lines LOW; external resistors pull them HIGH. CubeMX sets `GPIO_NOPULL` (correct when external pull-ups exist). Verify the GY-521 module has built-in 4.7kΩ pull-ups or add them externally.

**Diagnostic tip:** use `HAL_I2C_GetError()` to distinguish failure modes:
| Error code | Meaning |
|---|---|
| `0x04` (AF) | NACK — wrong address or device not powered |
| `0x01` (BERR) | Bus error — wiring fault |
| `0x20` (TIMEOUT) | Lines stuck LOW — short to GND |

---

#### 3. `#ifndef BUILD_TESTS` guard not firing in host test build

**Issue:** `mpu6050.h` uses `#ifndef BUILD_TESTS` to skip the `stm32f4xx_hal.h` include during host builds. The test build failed with `fatal error: stm32f4xx_hal.h: No such file or directory`.

**Root cause:** `BUILD_TESTS=ON` in `CMakePresets.json` is a CMake cache variable — it controls CMake logic but is never automatically forwarded to the C compiler as a preprocessor macro. So `#ifndef BUILD_TESTS` was always true.

**Fix:** Added to `CMakeLists.txt`:
```cmake
if(BUILD_TESTS)
    add_compile_definitions(BUILD_TESTS)  # passes -DBUILD_TESTS to the compiler
    ...
endif()
```

`add_compile_definitions` bridges the CMake variable to a compiler flag (`-DBUILD_TESTS`), making the preprocessor guard functional.

**Key takeaway:** CMake variables and C preprocessor macros are two separate namespaces. A CMake `option()` or cache variable is never visible to `#ifdef` unless explicitly forwarded via `add_compile_definitions` or `target_compile_definitions`.

---

## Phase 3 — Sensor Configuration & Raw Burst Read (2026-06-27)

### Decisions made

#### Accelerometer full-scale range: ±4g

Written to `ACCEL_CONFIG` register (`0x1C`), bits [4:3] = `01` → value `0x08`.

| AFS_SEL | Range | Sensitivity |
|---|---|---|
| 00 | ±2g | 16384 LSB/g |
| **01** | **±4g** | **8192 LSB/g** ← chosen |
| 10 | ±8g | 4096 LSB/g |
| 11 | ±16g | 2048 LSB/g |

**Why ±4g:** Vehicle events (braking, cornering) fall in the 0.3–0.8g range. ±2g gives no headroom; ±4g covers edge cases while keeping enough resolution (~820 counts difference between 0.4g and 0.5g).

---

#### Sample rate: 100 Hz via SMPLRT_DIV

The MPU-6050 does not expose sample rate directly. Instead, you supply a divider into `SMPLRT_DIV` register (`0x19`):

```
Sample Rate = Gyroscope Output Rate / (1 + SMPLRT_DIV)
```

With DLPF disabled (default), gyroscope output rate = **8000 Hz**.

```
100 Hz = 8000 / (1 + 79)  →  SMPLRT_DIV = 79
```

`MPU6050_SMPLRT_100HZ (79U)` stores the divider, not 100.

**Why 100 Hz:** Vehicle events last 200–500ms. At 100 Hz, a 200ms event yields 20 samples — enough to detect the onset reliably. Faster adds noise without benefit; slower risks missing sharp peaks.

**Note:** `SMPLRT_DIV` is 8-bit, so minimum achievable rate is 8000/256 ≈ 31 Hz.

---

#### Raw burst read: 6 bytes from ACCEL_XOUT_H (0x3B)

`HAL_I2C_Mem_Read` with length 6 starting at `0x3B`. The MPU-6050 auto-increments through:

```
0x3B ACCEL_XOUT_H │ 0x3C ACCEL_XOUT_L
0x3D ACCEL_YOUT_H │ 0x3E ACCEL_YOUT_L
0x3F ACCEL_ZOUT_H │ 0x40 ACCEL_ZOUT_L
```

One I2C transaction reads all three axes atomically.

---

#### Sensor noise is normal — data changing at rest is expected

On first run, raw Z readings varied (~0x21xx) while X and Y stayed near 0. This is correct:

- **Z ≈ 0x2100 (8448 counts) ≈ 1.03g** — gravity on the vertical axis ✓
- **X, Y ≈ 0** — sensor is flat ✓
- **Reading-to-reading variation (~±50 counts)** — MEMS sensor noise, not a bug

The MPU-6050 datasheet specifies ~400 μg/√Hz noise density. At 100 Hz that is ~4000 μg (~32 counts) of inherent noise. Phase 4 will address this with a moving average or IIR low-pass filter before feeding values into `evaluate_dynamics()`.

---

## Phase 4 — Data Acquisition & Signal Processing (2026-06-28)

### What was done

- Extended the main loop to compute g-force for all three axes (X, Y, Z), not just X
- Updated UART output to a human-readable format: `X:+0.003g  Y:-0.001g  Z:+1.002g`
- Fixed two bugs that prevented g-force from appearing on the terminal

---

### Bugs fixed

#### 1. `&x_axis_gforce` passed to `snprintf` instead of `x_axis_gforce`

`%f` expects a `double` passed **by value**. Passing the address of the float (`&x_axis_gforce`) causes `snprintf` to interpret the pointer bits as a floating-point number — the result is either garbage or an empty field. The fix is removing the `&`.

**Key takeaway:** `%f`/`%g`/`%e` are value specifiers. Never pass a pointer to a float to `snprintf` — unlike `scanf`, `printf`-family functions do not take addresses.

---

#### 2. `snprintf %f` producing blank output with `--specs=nano.specs`

Even after fixing the pointer bug, the g-force field remained blank. Root cause: the project links against **newlib-nano** (`--specs=nano.specs`), a size-optimised C library for embedded targets. Newlib-nano strips floating-point support out of `printf`/`snprintf` by default to save ~8 KB of flash. The `%f` specifier becomes a no-op.

**Fix:** add `-u _printf_float` to the linker flags in `cmake/gcc-arm-none-eabi.cmake`:

```cmake
set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} --specs=nano.specs -u _printf_float")
```

`-u _printf_float` is an *undefined symbol reference* — it forces the linker to pull in the full float-capable `printf` implementation from the library, overriding the stripped version.

**Key takeaway:** Newlib-nano is the default in most STM32 CMake templates. Any time `%f` prints blank or nothing, suspect the nano float stub. The fix is always `-u _printf_float` on the linker line.

---

### `%+.3f` format specifier explained

The UART format string uses `%+.3fg`:

| Part | Meaning |
|---|---|
| `%` | Start of format specifier |
| `+` | Always print the sign (`+` for positive, `-` for negative) |
| `.3` | 3 decimal places |
| `f` | Floating-point value |
| `g` | Literal character appended after the number |

The `+` flag does **not** hardcode a positive value — negative readings print `-` automatically. Forcing the sign to always appear makes it immediately obvious which direction an axis is accelerating, which matters when values are small and close to zero (e.g., `+0.003g` vs `-0.003g`).

---

### Gravity validation

At rest with the sensor flat:

- **Z ≈ +1.000g** — gravity acting on the vertical axis ✓
- **X, Y ≈ 0.000g** — no lateral or longitudinal acceleration ✓

Tilting the board 90° so X faces down shifts the reading to **X ≈ +1.000g, Z ≈ 0.000g**, confirming axis orientation and the g-force conversion are correct.



