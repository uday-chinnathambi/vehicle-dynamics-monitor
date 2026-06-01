# Vehicle Dynamics Monitor

## Brief
Vehicle Dynamics Monitor is a real-time, HAL-based embedded firmware component designed
to monitor vehicle motion using inertial sensor data. The firmware continuously evaluates
longitudinal and lateral acceleration to identify significant vehicle dynamics events such
as harsh braking, rapid acceleration, and aggressive cornering.

The software is developed using STM32CubeMX and STM32 HAL drivers, following a
production-oriented firmware structure suitable for automotive and embedded systems.

---

## Objective
The objective of this project is to implement a reliable and maintainable embedded firmware
function that:

- Interfaces with an inertial sensor using standard MCU peripherals
- Processes real-time motion data deterministically
- Converts raw sensor measurements into physical units
- Detects vehicle dynamics events based on defined thresholds
- Provides diagnostic output for validation and system integration

The project focuses on embedded firmware design practices rather than user-facing features,
mirroring the responsibilities of an ECU software module.

---

## Main Technical Parts

- **MCU Platform**
  - STM32 microcontroller configured via STM32CubeMX
  - Peripheral access through STM32 HAL drivers

- **Sensor Interface**
  - MPU-6050 accelerometer accessed over I2C
  - Register-level communication handled via HAL I2C APIs

- **Data Processing**
  - Reconstruction of signed 16-bit acceleration values
  - Conversion from raw sensor counts to g-force units

- **Application Logic**
  - Threshold-based evaluation of longitudinal and lateral acceleration
  - Classification of motion events relevant to vehicle dynamics

- **Diagnostics**
  - UART-based telemetry output for debugging and system validation
  - Human-readable messages intended for development-time monitoring

---

## How the System Works

1. **System Initialization**
   - Clock, GPIO, I2C, and UART peripherals are initialized using CubeMX-generated code
   - The inertial sensor is configured and taken out of low-power sleep mode

2. **Periodic Data Acquisition**
   - Acceleration data is read from the sensor at a fixed sampling interval
   - Relevant axis data is extracted for longitudinal and lateral motion analysis

3. **Signal Interpretation**
   - Raw accelerometer values are converted into physical acceleration units
   - Measurements are evaluated against predefined thresholds

4. **Event Detection**
   - Motion events are identified when acceleration exceeds configured limits
   - Events represent conditions such as harsh braking or aggressive cornering

5. **Diagnostic Reporting**
   - Detected events are transmitted via UART
   - Output can be consumed by development tools or higher-level systems

The firmware operates continuously, similar to a background monitoring task within an ECU.

---

## Engineering Decisions

- **HAL-Based Architecture**
  - STM32 HAL drivers are used to improve portability and long-term maintainability
  - Aligns with common production firmware development workflows

- **Deterministic, Threshold-Based Logic**
  - Chosen for predictable behavior and ease of validation
  - Suitable for safety-relevant embedded systems where determinism is critical

- **Modular Design**
  - Clear separation between hardware access and application logic
  - Enables future extension without large structural changes

- **UART Diagnostics**
  - Selected as a lightweight and robust method for development-time visibility
  - Reflects typical embedded validation and bring-up practices

---

## How to Run It

1. **Hardware Setup**
   - Connect the MPU-6050 sensor to the STM32 via I2C (SDA, SCL, VCC, GND)
   - Connect a USB-to-TTL adapter to the configured UART pins

2. **Firmware Deployment**
   - Open the project in STM32CubeIDE
   - Verify CubeMX configuration for the target board
   - Build and flash the firmware to the STM32

3. **Validation**
   - Open a serial terminal at 115200 baud
   - Reset the device to start execution
   - Tilt or move the sensor to simulate vehicle motion
   - Observe diagnostic messages indicating detected events

Testing can be performed without a vehicle by exploiting gravity-based sensor tilting.

---

## Limitations



---

## Future Work(plan)

- Introduce digital filtering (moving average or IIR) to reduce noise
- Replace blocking delays with non-blocking, time-based scheduling
- Add sensor calibration and bias compensation
- Abstract the motion detection logic into a reusable service layer
- Integrate RTOS support for task-based execution
- Support additional outputs (CAN, logging, or structured diagnostics)
- Extend detection logic to support scoring or severity classification

---

## Notes
This project is intentionally structured as a firmware component rather than a full system,
making it suitable as a foundation for more complex automotive or embedded applications.

