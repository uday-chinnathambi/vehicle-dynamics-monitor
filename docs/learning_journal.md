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
| **Platform** | Linux, macOS, Windows | Windows primary; Linux/macOS via STM32CubeIDE |
| **Standalone install** | Yes | Bundled with STM32CubeIDE or CubeProgrammer |

---

### Bottom Line

**OpenOCD** is the better fit for a VS Code + CMake workflow — it is platform-independent,
integrates cleanly with Cortex-Debug, and is not tied to ST's tooling.

**ST-Link GDB Server** is worth knowing about if you ever work inside STM32CubeIDE or need
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






