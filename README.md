# Bare-Metal AHRS (Attitude and Heading Reference System) on STM32

![STM32](https://img.shields.io/badge/MCU-STM32F446RE-blue)
![Language](https://img.shields.io/badge/Language-Bare--Metal%20C-orange)
![Sensor](https://img.shields.io/badge/Sensor-MPU6050-green)
![Filter](https://img.shields.io/badge/Filter-Mahony%20AHRS-red)

A robust, from-scratch embedded C project that implements an Attitude and Heading Reference System (AHRS) using the STM32F446RE microcontroller and an MPU6050 6-DOF IMU. 

This project completely bypasses the STM32 HAL/LL libraries, relying entirely on direct register-level manipulation to establish a custom I2C engine, hardware timers, and UART telemetry. It extracts raw accelerometer and gyroscope data, processes it through a Mahony filter via a strictly timed hardware interrupt, and streams real-time Roll, Pitch, and Yaw Euler angles to a host machine.

## 🧠 System Architecture

### The Data Pipeline
1. **Hardware I2C Engine (`I2C.c`):** A custom-built, open-drain I2C driver running on a 16MHz APB1 clock. It includes state-machine safety nets to detect bus lockups, ACK failures, and timeout errors without crashing the main CPU.
2. **TIM6 Hardware Interrupt (`TIM6.c`):** A strictly timed 100Hz interrupt routine. It fetches raw 16-bit integers from the MPU6050, converts them to physical units ($g$ forces and $rad/s$), and feeds them into the filter.
3. **Mahony Sensor Fusion (`MahonyAHRS.c`):** Computes the quaternion representation of the board's orientation in 3D space, heavily optimizing drift.
4. **FPU Accelerated Main Loop (`main.c`):** The Cortex-M4 Floating-Point Unit (FPU) is explicitly enabled at the register level to handle quaternion-to-Euler trigonometric math (arctan/arcsin) without pipeline stalls.
5. **UART Telemetry (`uart.c`):** Streams the calculated Roll, Pitch, and Yaw data at 115200 baud to the host machine for real-time graphical rendering.

## ⚡ Hardware Wiring

| MPU6050 Pin | STM32F446RE Pin | Description |
| :--- | :--- | :--- |
| **VCC** | 3V3 | Power (Do NOT use 5V) |
| **GND** | GND | Common Ground |
| **SCL** | PB8 (D15) | I2C Clock Line (Internal Pull-Up Enabled) |
| **SDA** | PB9 (D14) | I2C Data Line (Internal Pull-Up Enabled) |

## 🛠️ Key Features

* **Zero Abstraction Layer:** 100% bare-metal implementation using `stm32f4xx.h` CMSIS definitions.
* **Bulletproof I2C:** Handles Repeated Start conditions and shifting target addresses (`0x68` -> `0xD0`) at the hardware register level.
* **Context Protection:** Safe memory transfer of volatile quaternions from the interrupt context to the main loop using `__disable_irq()` to prevent data tearing.
* **macOS / UNIX Serial Compatibility:** Telemetry formatting optimized for UNIX `screen` output and Python `pyserial` ingestion.

## 🚀 Getting Started

### Prerequisites
* STM32CubeIDE (or standard ARM GNU Toolchain)
* An ST-LINK debugger
* A Host Machine (macOS/Linux/Windows) with a serial terminal

### Flashing the Board
1. Clone the repository.
2. Open the project in STM32CubeIDE.
3. Ensure the Core CMSIS headers and `stm32f4xx.h` are linked in your `C/C++ Build -> Settings -> Include Paths`.
4. Build the project to generate the `.elf` binary.
5. Flash to the Nucleo-64 board using the ST-LINK.

### Viewing Telemetry (macOS/Linux)
Open a terminal and connect to the Virtual COM port at 115200 baud.
```bash
# Find your specific USB modem port
ls /dev/tty.usb*

# Launch the screen monitor
screen /dev/tty.usbmodemXXXXX 115200
