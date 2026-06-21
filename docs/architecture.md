# Vehicle Dynamics Monitor — Architecture

## System Architecture

```mermaid
graph TD
    subgraph Hardware["Hardware Layer"]
        MPU["MPU-6050\n(I2C Sensor)"]
        STM["STM32 MCU\n(Nucleo-F446ZH)"]
        UART_HW["USB-TTL / UART"]
        MPU -->|SDA/SCL I2C| STM
        STM -->|TX/RX| UART_HW
    end

    subgraph Firmware["Firmware — ARM Target (build/)"]
        HAL["STM32 HAL / CMSIS\n(Drivers/)"]
        DRV["mpu6050.c\nSensor Driver\n(HAL-dependent)"]
        LOGIC["vehicle_dynamics.c\nEvent Detection\n(HAL-free, pure logic)"]
        LOG["uart_logger.c\nDiagnostics Output\n(HAL-dependent)"]
        MAIN["main.c\nAcquisition Loop"]

        HAL --> DRV
        HAL --> LOG
        DRV -->|"raw bytes → g-force (float)"| LOGIC
        LOGIC -->|"VehicleEvent enum"| LOG
        MAIN --> DRV
        MAIN --> LOGIC
        MAIN --> LOG
    end

    subgraph Tests["Test Build — Host GCC (build-test/)"]
        UNITY["Unity\nTest Framework"]
        UT_DYN["test_vehicle_dynamics.c\nEvent detection unit tests"]
        UT_MPU["test_mpu6050.c\nConversion unit tests"]

        UNITY --> UT_DYN
        UNITY --> UT_MPU
        UT_DYN -->|tests| LOGIC
        UT_MPU -->|tests| DRV
    end

    subgraph Build["Build System (CMake)"]
        TC_ARM["cmake/arm-none-eabi.cmake\nCross-compile toolchain"]
        TC_HOST["cmake/host-gcc.cmake\nHost toolchain"]

        TC_ARM -->|"cmake --preset Debug/Release"| Firmware
        TC_HOST -->|"cmake --preset test-debug"| Tests
    end

    STM -->|"reads sensor"| MPU
    UART_HW -->|"event messages"| Terminal["Serial Terminal\n(Developer PC)"]
```

---

## Data Flow

```mermaid
sequenceDiagram
    participant MPU as MPU-6050
    participant DRV as mpu6050.c
    participant LOGIC as vehicle_dynamics.c
    participant LOG as uart_logger.c
    participant TTL as Serial Terminal

    loop Every poll interval (HAL_GetTick)
        DRV->>MPU: I2C burst read (6 bytes)
        MPU-->>DRV: Raw accel bytes (X, Y, Z)
        DRV->>DRV: reconstruct_raw_value() → signed int16
        DRV->>DRV: raw_to_gforce() → float [g]
        DRV->>LOGIC: evaluate_dynamics(ax, ay, az)
        LOGIC-->>DRV: VehicleEvent enum
        LOGIC->>LOG: log_event(VehicleEvent)
        LOG->>TTL: UART string (e.g. "HARSH_BRAKING")
    end
```

---

## Layer Boundaries

```mermaid
graph LR
    subgraph Host_Testable["Host-Testable (no HAL)"]
        VD["vehicle_dynamics.c\nfloat in → VehicleEvent out"]
        CONV["mpu6050 conversion fns\nraw bytes → g-force"]
    end

    subgraph HAL_Dependent["HAL-Dependent (target only)"]
        I2C["I2C read/write\n(HAL_I2C_*)"]
        UART["UART transmit\n(HAL_UART_*)"]
    end

    I2C --> CONV
    CONV --> VD
    VD --> UART
```

> **Key architectural rule:** `vehicle_dynamics.c` contains zero HAL calls. Pure `float` in, `VehicleEvent` enum out. This boundary is what makes TDD on the host possible without mocking the entire HAL.

---

## CMake Build Configurations

```mermaid
graph TD
    ROOT["CMakeLists.txt"]
    ROOT -->|"BUILD_TESTS=OFF\ncmake --preset Debug"| ARM["ARM Target Build\nbuild/\narm-none-eabi-gcc\nOutputs: firmware.elf"]
    ROOT -->|"BUILD_TESTS=ON\ncmake --preset test-debug"| HOST["Host Test Build\nbuild-test/\nhost gcc\nOutputs: test executables"]
    ARM --> FLASH["Flash to STM32\nvia OpenOCD / ST-Link"]
    HOST --> RUN["Run on Developer PC\n./build-test/tests/..."]
```
