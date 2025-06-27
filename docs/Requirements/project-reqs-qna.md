# Multi Sonicator I/O Controller - Project Requirements Q&A

## **Business Context**

### 1. Project Name/Codename

**Multi Sonicator I/O Controller**

### 2. Timeline

**Target delivery date: 2 weeks from now**

### 3. Budget

- **Development Budget**: $400
- **Target Unit Cost**: As much as we can get

### 4. Market Context

This is an extension to the Automation System for Cannasol Technologies, it will be sold as an add-on.

### 5. Sonicator Specifications

- **Quantity**: Up to 4 sonicators
- **Model**: CT2000 sonicator
- **Frequency Ranges**: Need to research CT2000 specifications
- **Power Levels**: Need to research CT2000 specifications

### 6. Performance Requirements

- **Response Time**: Faster than MODBUS communication protocol requirements so the bottleneck is the MODBUS
- **Amplitude Control**: 12-bit resolution works
- **Frequency Output**: 100Hz accuracy (first three digits only)
  - Example: frequency of 19876 could be output as 19900

## **System Integration**

### 7. Main Automation System

- **System**: Proprietary custom system built by Cannasol
- **PLC**: Velocio 1630c PLC
- **HMI**: Connected to PLC over MODBUS
- **Cloud Interface**: ESP32 (WiFi-adapter) connected to HMI via MODBUS
- **Communication Protocols**:
  - MODBUS: PLC ↔ HMI and HMI ↔ ESP32
  - HTTP requests: ESP32 ↔ Firebase Cloud Database
  - SSL Client streaming: Firebase realtime database updates
- **Current Setup**: Spare DB9 port on main automation box currently controls/monitors one CT2000 sonicator
- **Planned Change**: Modify DB9 port to communicate with multiple sonicator controller, which will individually control sonicators

### 8. ESP32 Integration

- **Current Status**: ESP32 cloud integration already implemented
- **Mobile App**: Already implemented
- **Required Extensions**:
  - Extend mobile app for multiple sonicator UI components
  - Extend HMI for multiple sonicator monitoring and control

## **Development Team & Resources**

### 9. Team Composition

- **Lead Developer**: Stephen Boyett (sole developer)
- **Hardware Design Support**: Josh (CEO)
- **Team Type**: All internal

### 10. Development Infrastructure

- **Test Equipment**: Limited - only hardware used in automation system
- **PCB Design Tool**: EasyEDA PCB designer
- **Testing Resources**: Sonicators available for testing
- **Current Test Setup**: Automation system connected to four sonicators (currently monitoring only one)

## **Regulatory & Compliance**

### 11. Regulatory Requirements

- **Market**: US Market only for now
- **Application**: Industrial liquid sonic processing for nano emulsions
- **Target Industries**: Drink active ingredients (active mushrooms, THC/CBD)
- **CE Marking**: Question - What is CE Marking?

### 12. Quality Standards

- **Certifications**: No specific quality certifications required
- **Quality Goal**: Highest quality we can deliver quickly

## **Production & Manufacturing**

### 13. Volume Expectations

- **Prototype**: One prototype for in-house use and testing
- **Production**: Annual volume ~12 units
- **Variants**: No plans for multiple variants
- **Testing Strategy**: Use local system for testing new expansions before customer sales

### 14. Manufacturing Strategy

- **Assembly**: In-house assembly of controller adapter
- **Outsourced**: Enclosure and PCB manufacturing

## **Risk Management**

### 15. Critical Dependencies

- **Suppliers**: Not determined yet
- **Long-lead Components**: Unknown at this time

### 16. Technical Risks

- **Integration Challenge**: Need to ensure PLC has access to all values
- **Architecture Options**:
  - Option 1: Integrate controller as second slave in MODBUS network (PLC ↔ HMI)
  - Option 2: Single pin communication protocol between PLC and controller (only freq lock, overload, power)
  - **Preference**: MODBUS integration preferred
- **System Transition**: Switching from single sonicator to multiple sonicator system
- **Compatibility Requirement**: Automatic interchangeability detection or graceful handling of both single/multiple configurations

## **Technical Clarifications & Answers**

### **1. CT2000 Interface Pin Assignments**

**Q:** Pin 2 (Overload Reset) availability and Pin 8 (Amplitude Control) internal switch requirements?
**A:**

- We will need to confirm the Overload Reset Pin 2 is actually available for overload reset control
- We will have to verify the existence of the internal amplitude control switch and verify functionality
- Add verification requirements to project overview

### **2. Power Supply Architecture**

**Q:** Dedicated 24VDC supply vs DB9 power distribution?
**A:** We are using DB9 power delivery from the Main Automation Box internal power supply. The Main Box 24VDC supply provides power through DB9 Pin 1, with ground return on Pin 4. Controller performs local regulation (24V→12V→5V) internally.

### **3. MODBUS Network Hierarchy**

**Q:** What is the exact MODBUS master/slave relationship?
**A:** The HMI acts as the Master for the network with both the PLC and the Multi Sonicator Controller as slaves, as well as master for a separate network with the ESP32 WiFi adapter.

### **4. ATmega32A GPIO Pin Allocation**

**Q:** Sufficient pins for all interfaces?
**A:** 40 pins total vs 39 pins required = sufficient capacity. Need to add a diagram of the ATmega32A with all pins labeled to confirm capabilities.

### **5. Frequency Measurement Implementation**

**Q:** How to handle 4 simultaneous frequency measurements?
**A:**

- Implementation details to be researched during development phase
- Four pins counting ticks of frequency outputs
- Should be easily monitored with 16MHz clock since max sonicator frequency is 21kHz
- Output is divided by 10, so maximum counts = 2100/second
- Open to external frequency-to-voltage converters if they are cheap, easy to implement, and improve functionality

### **6. Component Budget**

**Q:** Is $220 budget realistic for enhanced features?
**A:** Raise the budget to $400 for development costs.

### **7. Real-Time Performance Requirements**

**Q:** Can response time faster than MODBUS protocol requirements be achieved?
**A:**

- Response time requirement: faster than MODBUS communication protocol requirements so the bottleneck is the MODBUS
- Interrupt priority scheme not yet defined
- MODBUS communication will be the bottleneck and primary time constraint

### **8. Integration with Existing System**

**Q:** Backward compatibility and fallback options?
**A:**

- DB9 cable from main box to single sonicator will be replaced with cable to controller
- Backward compatibility testing plan needs to be devised
- Open to fallback options - considering redundancy where if one chip fails, system automatically switches to backup
- Need ideas for redundancy implementation
