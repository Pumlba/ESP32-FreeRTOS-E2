# ESP32-FreeRTOS-E2
# ESP32 FreeRTOS – Temperature Monitoring and Alarm System

## 1. Project Overview

This project implements a temperature monitoring and alarm system using an ESP32 microcontroller and FreeRTOS.

A potentiometer is used to simulate a temperature sensor. The ESP32 reads the analog value (ADC), converts it into a temperature value from 0°C to 100°C, and controls an LED and buzzer based on the system state.

The project is simulated using **Wokwi** and programmed with the **Arduino Framework**.

## 2. Hardware Components

| Component | ESP32 Pin | Function |
|---|---|---|
| ESP32 DevKit C V4 | - | Main controller |
| Potentiometer | GPIO 34 | Simulates temperature sensor |
| Red LED + 220Ω resistor | GPIO 18 | Visual alarm |
| Buzzer | GPIO 19 | Sound alarm |

## 3. System Architecture

The system uses two FreeRTOS tasks:

- **ReadSensor (Priority 1):**
  - Reads ADC data from GPIO 34.
  - Converts ADC values into temperature (0–100°C).
  - Sends sensor data to the Queue.
  - Runs periodically every 100 ms.
  - Records Period, Jitter, and Execution Time.

- **Control (Priority 2):**
  - Receives sensor data from the Queue.
  - Checks temperature and data validity.
  - Controls the LED and buzzer.
  - Detects Queue timeout and invalid sensor data.

### Synchronization Mechanisms

- **Queue:** Stores up to 5 sensor samples and transfers data between tasks.
- **Mutex:** Protects Serial Monitor output to prevent log messages from overlapping.

## 4. Operating States

| State | Condition | LED | Buzzer |
|---|---|---|---|
| NORMAL | Normal temperature | OFF | OFF |
| ALARM | Temperature > 50°C | ON | ON |
| ALARM (Hold) | Temperature > 45°C while already in ALARM | ON | ON |
| FAULT | Queue timeout or invalid sensor data | ON | ON |

The system uses a 45°C recovery threshold to prevent frequent switching between NORMAL and ALARM states.

## 5. Testing Scenarios

| Test Case | Description | Result |
|---|---|---|
| Test 1 | Normal and alarm temperature conditions | PASS |
| Test 2 | Mutex contention (Control holds Mutex for an additional 5 ms) | PASS |
| Test 3 | Queue timeout (ReadSensor stops after 10 seconds) | PASS |
| Test 4 | Invalid sensor data at low and high temperatures | PASS |

## 6. Project Files

- `sketch.ino` – Main ESP32 source code.
- `diagram.json` – Wokwi circuit configuration.
- `README.md` – Project documentation.

## 7. Simulation

**Simulation platform:** Wokwi

**Wokwi Project Link:** [Insert your Wokwi project link here]

## 8. Author

**Project:** E2 – ESP32 and FreeRTOS  
**Repository:** ESP32-FreeRTOS-E2
