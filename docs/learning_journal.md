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





