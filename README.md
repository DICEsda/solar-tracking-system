# IoT-Based Dual-Axis Solar Tracking System

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform: ESP32](https://img.shields.io/badge/Platform-ESP32-blue.svg)](https://www.espressif.com/en/products/socs/esp32)
[![Framework: Arduino](https://img.shields.io/badge/Framework-Arduino-00979D.svg)](https://www.arduino.cc/)

An intelligent IoT solar tracking system that optimizes solar energy production by dynamically adjusting panel orientation based on real-time light sensor data and environmental conditions.

![Solar Tracking System](docs/images/system-overview.png)

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [System Architecture](#system-architecture)
- [Hardware Requirements](#hardware-requirements)
- [Installation](#installation)
- [Usage](#usage)
- [Project Structure](#project-structure)

## Overview

The IoT-Based Dual-Axis Solar Tracking System (IoT-DASTS) is an embedded systems project designed to maximize solar panel efficiency through intelligent sun tracking. The system uses four light sensors positioned in cardinal directions to determine optimal panel orientation, adjusting both horizontal (stepper motor) and vertical (servo motor) axes in real-time.

### Key Highlights

- **Real-time sun tracking** using 4-directional light sensors
- **Dual-axis control** for horizontal and vertical panel adjustment
- **Environmental monitoring** with temperature and humidity sensors (HTU21D)
- **Web-based dashboard** for remote monitoring and visualization
- **Custom Linux kernel driver** for precise motor control
- **ESP32-based IoT connectivity** with WiFi capabilities

## Features

### Hardware Features
- 4-axis light intensity sensing (Left, Right, Up, Down)
- HTU21D temperature and humidity monitoring
- Servo motor for vertical tilt adjustment (0-180°)
- Stepper motor for horizontal rotation
- TFT display for local data visualization
- ESP32 microcontroller with WiFi

### Software Features
- Real-time sensor data acquisition
- Asynchronous web server with REST API
- Live data graphing with Highcharts
- Custom Linux device driver for motor control
- RTOS task management for concurrent operations
- Responsive web interface

## System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                     ESP32 (Main Controller)                  │
│  ┌────────────┐  ┌──────────────┐  ┌──────────────┐        │
│  │ Light      │  │ HTU21D       │  │ TFT Display  │        │
│  │ Sensors x4 │  │ (Temp/Humid) │  │              │        │
│  └────────────┘  └──────────────┘  └──────────────┘        │
│         │                │                  │               │
│         └────────────────┴──────────────────┘               │
│                          │                                   │
│                   ┌──────▼───────┐                          │
│                   │  Web Server  │◄────────────────┐        │
│                   │  (AsyncWeb)  │                 │        │
│                   └──────┬───────┘                 │        │
└──────────────────────────┼─────────────────────────┼────────┘
                           │                         │
                           │ UART                    │ WiFi
                           │                         │
┌──────────────────────────▼─────────────────────────▼────────┐
│               Linux System (Raspberry Pi)                    │
│  ┌─────────────────────────────────────────────────────┐    │
│  │          Kernel Driver (plat_drv)                   │    │
│  │  ┌──────────────┐         ┌──────────────┐         │    │
│  │  │ Servo Driver │         │Stepper Driver│         │    │
│  │  │  (GPIO 18)   │         │ (GPIO 22-25) │         │    │
│  │  └──────┬───────┘         └──────┬───────┘         │    │
│  └─────────┼────────────────────────┼─────────────────┘    │
└────────────┼────────────────────────┼──────────────────────┘
             │                        │
       ┌─────▼──────┐           ┌────▼─────┐
       │   Servo    │           │ Stepper  │
       │   Motor    │           │  Motor   │
       └────────────┘           └──────────┘
```

## Hardware Requirements

### ESP32 Module
- **Board**: LilyGO T-Display ESP32 (or compatible ESP32-WROOM-32)
- **Flash**: 4MB
- **RAM**: 520KB
- **WiFi**: 802.11 b/g/n

### Sensors
- **HTU21D**: I2C Temperature and Humidity Sensor
- **Light Sensors**: 4x Analog photoresistors/photodiodes
  - GPIO32 (Left)
  - GPIO33 (Right)
  - GPIO39 (Up)
  - GPIO36 (Down)

### Actuators
- **Servo Motor**: Standard 50Hz PWM servo (0-180°)
- **Stepper Motor**: 28BYJ-48 or similar 4-phase unipolar stepper

### Linux System (Optional)
- **Raspberry Pi 3/4** (or compatible ARM-based Linux system)
- **Kernel Version**: 5.4.83 or compatible

### Display
- **TFT Display**: 135x240 ST7789V (integrated with LilyGO T-Display)

## Installation

### 1. Clone the Repository

```bash
git clone https://github.com/yourusername/solar-tracking-system.git
cd solar-tracking-system
```

### 2. ESP32 Setup

#### Configure WiFi Credentials
Edit `esp32/include/Wifi_Config.h` or modify the main.cpp setup:

```cpp
HandleWiFi_init("YourSSID", "YourPassword");
```

#### Build and Upload with PlatformIO

```bash
cd esp32
pio run --target upload
pio device monitor
```

Or use PlatformIO IDE in VSCode.

### 3. Linux Driver Setup (Optional)

#### Prerequisites
Install kernel headers and build tools:

```bash
sudo apt-get update
sudo apt-get install raspberrypi-kernel-headers build-essential
```

#### Build the Kernel Module

```bash
cd linux-driver
make modules
```

#### Install the Driver

```bash
sudo make modules_install
# Load the device tree overlay
sudo dtoverlay Servo-Stepper.dtbo
# Verify device files
ls -l /dev/plat_drv*
```

## Usage

### Starting the System

1. **Power on the ESP32** - The system will automatically:
   - Connect to WiFi
   - Initialize sensors
   - Start the web server
   - Begin monitoring light levels

2. **Access the Web Dashboard**
   - Connect to the same WiFi network
   - Open a browser and navigate to: `http://<ESP32_IP_ADDRESS>`
   - The IP address is displayed on the TFT screen

### Web Interface Features

- **Real-time Graphs**: View temperature and humidity trends
- **Sensor Readings**: Monitor current light intensity from all 4 sensors
- **Sun Direction**: See which direction has maximum light intensity
- **System Status**: Check connection and sensor health

### Local Display

The TFT display shows:
- WiFi connection status and IP address
- Temperature and humidity readings
- Light sensor values
- Sun direction indicator

### API Endpoints

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/` | GET | Main dashboard HTML |
| `/temperature` | GET | Current temperature (°C) |
| `/humidity` | GET | Current humidity (%) |
| `/graph_Temp` | GET | Temperature data for graphing |
| `/graph_Humidity` | GET | Humidity data for graphing |

## Project Structure

```
solar-tracking-system/
├── esp32/                          # ESP32 firmware
│   ├── include/                    # Header files
│   │   ├── DisplayHandler.h        # TFT display management
│   │   ├── Endpoints.h             # Web server HTML & endpoints
│   │   ├── HTU.h                   # Temperature/humidity sensor
│   │   ├── Lys.h                   # Light sensor management
│   │   └── Wifi_Config.h           # WiFi configuration
│   ├── lib/                        # External libraries
│   │   └── HTU21D_Sensor_Library-1.0.2/
│   ├── src/                        # Source code
│   │   └── main.cpp                # Main application
│   ├── platformio.ini              # PlatformIO configuration
│   └── .gitignore
│
├── linux-driver/                   # Linux kernel driver
│   ├── Servo-Stepper.c             # Kernel module source
│   ├── Servo-Stepper.dts           # Device tree source
│   ├── main.c                      # User-space test program
│   ├── Makefile                    # Build configuration
│   └── README.md                   # Driver documentation
│
├── docs/                           # Documentation
│   ├── images/                     # Diagrams and photos
│   ├── architecture.md             # System architecture details
│   └── api-reference.md            # API documentation
│
├── .gitignore                      # Git ignore file
├── LICENSE                         # MIT License
└── README.md                       # This file
```

## Pin Configuration

### ESP32 Pin Mapping

```cpp
// I2C Bus (HTU21D)
#define SDA_PIN 21
#define SCL_PIN 22

// Light Sensors (ADC)
#define LIGHT_LEFT    32  // ADC1_CH4
#define LIGHT_RIGHT   33  // ADC1_CH5
#define LIGHT_UP      39  // ADC1_CH3
#define LIGHT_DOWN    36  // ADC1_CH0

// UART (to Linux system)
#define RX_PIN 27
#define TX_PIN 26
```

### Raspberry Pi GPIO Mapping

```
GPIO 18  → Servo Motor PWM
GPIO 22  → Stepper Motor Phase 1
GPIO 23  → Stepper Motor Phase 2
GPIO 24  → Stepper Motor Phase 3
GPIO 25  → Stepper Motor Phase 4
```
