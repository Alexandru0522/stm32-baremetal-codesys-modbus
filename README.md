# STM32 Bare-Metal C & CODESYS SoftPLC Modbus RTU

Industrial automation demo featuring an **STM32 Microcontroller** acting as a **Modbus RTU Server (Slave ID 1)** communicating over UART/RS-485 with a **CODESYS V3 SoftPLC (Client / Master)** and TargetVisu HMI.

## Architecture & Features
- **STM32 Bare-Metal C Firmware**:
  - Custom UART register-level driver
  - Modbus RTU protocol handling (FC03 Read Holding Registers, FC06 Write Single Register)
  - Hardware mapping: LED control & ADC potentiometer reading
- **CODESYS SoftPLC Application**:
  - Modbus RTU Client configuration
  - Channel mapping & I/O mapping to PLC_PRG
  - TargetVisu HMI with push-buttons and indicator lamps

## Repository Structure
- `stm32/` - Bare-metal C source code, linker script, startup file, and Makefile.
- `plc/` - CODESYS project file (`.project`) and full portable package (`.projectarchive`).
