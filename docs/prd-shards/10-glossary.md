# 10. Glossary

* **CT2000:** The specific model of the sonicator unit being controlled by the multiplexer.
* **Firmware:** Software embedded within the microcontroller, responsible for all control logic and communication.
* **HAL (Hardware Abstraction Layer):** A software layer that isolates the core application logic from the specific microcontroller hardware, improving portability.
* **HIL (Hardware-in-the-Loop):** A testing methodology where the final firmware is run on the actual microcontroller while simulating the external hardware (sonicators, PLC) to validate real-world interactions.
* **HMI (Human-Machine Interface):** The graphical interface on the PLC system used by operators to control and monitor the sonicators.
* **MODBUS RTU:** A master-slave serial communication protocol used for industrial control systems. The multiplexer acts as the slave device.
* **Multiplexer:** The hardware device, controlled by the firmware, that manages communication and control signals for up to four sonicator units.
* **PLC (Programmable Logic Controller):** The industrial computer that acts as the master controller, sending commands to and receiving data from the multiplexer via MODBUS.
* **Sonicator:** A device that uses ultrasonic energy to process materials.
* **Telemetry:** Data related to the status and performance of the sonicators (e.g., power, temperature, status flags) transmitted from the multiplexer to the PLC.
* **Watchdog Timer:** A hardware safety feature that resets the microcontroller if the main application freezes or becomes unresponsive.
