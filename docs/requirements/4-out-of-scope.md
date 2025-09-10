# 4. Out of Scope

The following items are explicitly excluded from the scope of this project:

* **PLC/HMI Development**: The design, development, and testing of the PLC logic and HMI screens that will consume the MODBUS interface are the responsibility of the integration team and are not included in this project.
* **Hardware Design and Manufacturing**: This project covers the firmware for the specified ATmega32A-based multiplexer. The design, fabrication, and sourcing of the physical hardware are considered complete and outside the scope of this firmware development effort.
* **Cloud Connectivity and Data Logging**: While future enhancements may include cloud features, the initial version of this project will not implement any form of internet connectivity, cloud data streaming, or remote telemetry logging.
* **Advanced Power Analytics**: The system will report real-time power consumption (Watts). Any further analysis, such as historical trending, efficiency calculations, or predictive maintenance analytics, is not in scope.
* **Dynamic Reconfiguration**: The system is designed to control up to four pre-defined sonicator units. It will not support dynamic addition or removal of units at runtime.
