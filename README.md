# Bare-Metal AHRS (Attitude and Heading Reference System) on STM32

![STM32](https://img.shields.io/badge/MCU-STM32F446RE-blue)
![Language](https://img.shields.io/badge/Language-Bare--Metal%20C-orange)
![Sensor](https://img.shields.io/badge/Sensor-MPU6050-green)
![Filter](https://img.shields.io/badge/Filter-Mahony%20AHRS-red)

![AHRS Demo GIF](link_to_your_gif.gif)
*(Caption: Real-time Roll, Pitch, and Yaw telemetry calculated via Mahony AHRS and 100Hz hardware interrupts.)*

A robust, from-scratch embedded C project...


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

## ⚠️ Challenges Faced & Solutions

Building a bare-metal pipeline without HAL abstracts away the safety nets, exposing raw silicon quirks. Here are the key technical hurdles overcome during development:

* **The Cortex-M4 FPU HardFault Trap:** * *Challenge:* The STM32F446RE features a hardware Floating-Point Unit (FPU), but it is disabled by default upon reset. When the Mahony filter attempted to execute float-heavy trigonometric operations (`sqrtf`, `asinf`), the core threw a fatal HardFault exception, instantly freezing the microcontroller.
  * *Solution:* Explicitly activated the CP10 and CP11 coprocessors by setting the appropriate bits in the System Control Block (`SCB->CPACR`) at the very first line of `main()`, unlocking hardware-accelerated float math.
* **Repeated Start & Register Clearing Lockups:** * *Challenge:* When executing multi-byte burst reads (fetching 14 bytes of raw Accel/Gyro data), the I2C bus would permanently hang if the `ADDR` flag wasn't cleared at the exact right nanosecond before sending the Repeated Start condition.
  * *Solution:* Implemented a bulletproof, register-level sequence that precisely reads `SR1` and `SR2` to clear flags, and manually disables the `ACK` bit exactly one byte before the `STOP` condition is generated.
* **Open-Drain Bus Paralysis:** * *Challenge:* The I2C bus remained completely silent (`SR1: 0`, `SR2: 0`) because I2C is an open-drain protocol and the lines were sitting at 0V, tricking the STM32 into thinking the bus was perpetually busy.
  * *Solution:* Explicitly configured the `GPIOB->PUPDR` registers to engage the STM32's internal pull-up resistors on the SCL and SDA pins, pulling the idle bus up to 3.3V.
* **Context Tearing in the Mahony Pipeline:** * *Challenge:* The 100Hz `TIM6` hardware interrupt was updating the global quaternion array at the exact moment the main loop was trying to read it for Euler angle conversion, leading to corrupted trigonometric math.
  * *Solution:* Wrapped the quaternion array copy operation inside the main loop with `__disable_irq()` and `__enable_irq()` to briefly pause the interrupt, ensuring thread-safe data transfers without missing a clock cycle.
 

    
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

#Hit the black reset button
```
## 🚀 Future Scope & Strategic Roadmap

This bare-metal AHRS foundation serves as the core sensory unit for more complex, industrial-grade autonomous systems. The development roadmap is structured into the following strategic phases:

### Phase 1: Hardware Hardening & PCB Design
* **Objective:** Transition from a breadboard prototype to a robust, industrial-grade hardware platform.
* **Action Items:** * Design a custom 4-layer to 8-layer PCB to minimize EMI, ensure signal integrity for the high-speed I2C lines, and optimize the ground plane.
  * Integrate the STM32F446RE footprint directly onto the board alongside the MPU6050, complete with onboard buck converters and power regulation.

### Phase 2: Deterministic Task Management (RTOS)
* **Objective:** Move beyond bare-metal hardware timers to a scalable, multi-tasking software architecture.
* **Action Items:**
  * Port the current super-loop and interrupt architecture to FreeRTOS.
  * Separate the I2C polling, Mahony filter calculations, and UART telemetry into dedicated, priority-managed tasks, utilizing mutexes to protect the quaternion data arrays.

### Phase 3: Autonomous Systems & SLAM
* **Objective:** Elevate the module from a passive sensor to an active node in a robotics ecosystem.
* **Action Items:**
  * Translate the C telemetry pipeline into a **micro-ROS** node.
  * Publish the `sensor_msgs/Imu` topic over a serial transport to be ingested by a larger ROS 2 SLAM (Simultaneous Localization and Mapping) architecture for smart robots.

### Phase 4: Edge Intelligence (TinyML)
* **Objective:** Execute predictive models directly on the STM32 silicon to reduce telemetry bandwidth.
* **Action Items:**
  * Deploy a TinyML model on the microcontroller to process the raw IMU tensor data locally.
  * Implement real-time anomaly detection (e.g., detecting specific vibration signatures or mechanical wear patterns) directly at the edge.

### Phase 5: Scaled Infrastructure Deployment
* **Objective:** Package the hardware for large-scale environmental or industrial monitoring.
* **Action Items:**
  * Integrate additional acoustic or cross-correlation sensors alongside the IMU.
  * Develop a communication protocol to allow multiple nodes to sync, forming the backbone of automated feedback systems for campus-level infrastructure tracking or pipeline monitoring.

---

## 👨‍💻 About the Author

**Kavish Sharma** *B.Tech in Electronics and Instrumentation Engineering | NIT Silchar*

Passionate about bare-metal embedded systems, industrial PCB design, and building the core sensory architecture for autonomous robotics. 

📫 **Let's Connect:**
* **GitHub:** [@Kavish-1704](https://github.com/Kavish-1704)
* **LinkedIn:** [(https://www.linkedin.com/in/kavish-sharma-724168314/)]
* **Email:** [kavishsharma5757@gmail.com]

---
*If you found this project interesting or helpful, feel free to drop a ⭐ on the repository!*
