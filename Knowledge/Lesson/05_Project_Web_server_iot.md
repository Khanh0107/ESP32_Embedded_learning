# ESP32 Web Server IoT Project

---

# 1. Project Overview

This project implements a simple embedded Web Server running on an ESP32.  
The system allows users to monitor environmental data and control hardware devices through a web interface.

Main functionalities:

- Display real-time Temperature and Humidity measured from DHT11 sensor
- Show historical temperature and humidity charts using ThingSpeak Cloud
- Control an LED (ON/OFF) from web interface
- Adjust LED brightness using PWM slider
- Control RGB color of WS2812B LED strip
- Change WiFi SSID and password via web interface

The system demonstrates practical implementation of:

- WiFi Station mode
- HTTP Server (ESP-IDF)
- Raw HTTP Client (socket-based)
- Cloud integration (ThingSpeak)
- JSON parsing (cJSON)
- RMT driver for WS2812B

---

# 2. System Architecture

## 2.1 Components

### Hardware

- ESP32
- DHT11 Temperature & Humidity Sensor
- Single LED (PWM controlled)
- WS2812B RGB LED Strip (8 LEDs)

# ESP32 Web Server — IoT Example

A compact guide for an ESP32-based web server that monitors DHT11 temperature/humidity, integrates with ThingSpeak, and controls LEDs (single PWM LED and WS2812B RGB strip).

---

## Table of contents

- [Project Overview](#project-overview)
- [System Architecture](#system-architecture)
  - [Components](#components)
  - [Data Flow](#data-flow)
  - [Key Features](#key-features)
- [Directory Structure](#directory-structure)
- [Build and Run](#build-and-run)
- [Implementation Notes](#implementation-notes)
- [Limitations & Future Improvements](#limitations--future-improvements)
- [Learning Outcomes](#learning-outcomes)

---

## Project Overview

This project implements a simple HTTP web server on an ESP32 that:

- Reads temperature and humidity from a DHT11 sensor
- Sends/receives data to/from ThingSpeak for historical charts
- Provides a web UI to toggle a single LED, adjust PWM brightness, and control an 8-LED WS2812B strip
- Allows updating WiFi credentials via the web interface

It showcases WiFi (station mode), an ESP-IDF HTTP server, a socket-based HTTP client, JSON parsing with cJSON, and RMT-driven WS2812B control.

---

## System Architecture

### Components

- Hardware
  - ESP32
  - DHT11 temperature & humidity sensor
  - Single LED (PWM)
  - WS2812B RGB LED strip (8 LEDs)

- Software modules
  - WiFi manager (station mode)
  - HTTP server (esp_http_server)
  - HTTP client (BSD sockets) for ThingSpeak
  - DHT11 driver
  - LEDC (PWM) driver
  - WS2812B driver (RMT)
  - JSON parsing (cJSON)
  - FreeRTOS tasks

### Data flow

Sensor monitoring:

- ESP32 samples DHT11 periodically (e.g., every 1s)
- Samples stored in memory
- Browser requests `/getdatadht11` → ESP32 returns JSON

ThingSpeak integration:

- Upload: ESP32 POSTs temperature/humidity every ~20s using a raw TCP socket and `application/x-www-form-urlencoded` format
- Download: ESP32 GETs latest fields, parses JSON via cJSON, and may change local outputs (e.g., LED) based on thresholds

Web control flow:

Browser → HTTP request → ESP32 → hardware action

Common URIs:

| URI | Method | Description |
| --- | ------ | ----------- |
| `/switch1` | POST | Toggle single LED ON/OFF |
| `/slider` | POST | Adjust PWM duty (brightness) |
| `/rgb?color=RRGGBB` | GET | Set WS2812B color (hex) |
| `/wifiinfo` | POST | Update WiFi SSID/password |

### Key features

- Real-time sensor monitoring
- Cloud visualization via ThingSpeak
- Remote LED and RGB strip control
- Dynamic WiFi reconfiguration from web UI
- Multitasking with FreeRTOS
- Low-level HTTP client (BSD sockets)
- JSON parsing using cJSON
- RMT-based WS2812B control

---

## Directory Structure

project_root/

- common/
  - dht11/
    - dht11.h
    - dht11.c
  - http_server_app/
    - http_server_app.h
    - http_server_app.c
  - input_iot/
    - input_iot.h
    - input_iot.c
  - ledc_app/
    - ledc_app.h
    - ledc_app.c
  - led_strip/
    - include/
        - led_strip.h
    - src/
        - led_strip.c
  - output_iot/
    - output_iot.h
    - output_iot.c
  - ws2812b/
    - ws2812b.h
    - ws2812b.c
- main/
  - app_main.c
- html/
  - index.html  
- CMakeLists.txt
- sdkconfig
- README.md


---

## Build and Run

### Requirements

- ESP-IDF (appropriate version) installed and sourced
- Python 3.x
- USB driver for your ESP32 board

### Quick steps

1. Configure the project (optional: WiFi credentials):

```bash
idf.py menuconfig
```

2. Build:

```bash
idf.py build
```

3. Flash (replace `COMx` with your port):

```bash
idf.py -p COM8 flash monitor
```

4. Monitor serial output:

```bash
idf.py monitor
```

## Code reference

- [Code reference](https://github.com/Khanh0107/ESP32_Embedded_learning/tree/main/04_station)

---

## Implementation Notes

- WiFi
  - Station mode with event group to track connection state
  - Retry mechanism implemented

- HTTP server
  - Uses `esp_http_server`
  - Registers URI handlers and supports modular callbacks
  - Includes a 404 handler

- HTTP client (ThingSpeak)
  - Implements BSD socket API with `getaddrinfo()` for DNS
  - Constructs HTTP headers manually
  - Blocking reads and connection-per-request behavior

- WS2812B (RMT)
  - Uses the RMT peripheral; note GRB vs RGB ordering depending on strip

- FreeRTOS tasks
  - Separate tasks for ThingSpeak GET/POST and DHT11 sampling

- Memory
  - cJSON allocations use heap; ensure `cJSON_Delete()` and `free()` where needed
  - Socket buffers typically sized 512–1024 bytes

---

## Limitations

- No HTTPS support (no TLS)
- No authentication on the web server
- Blocking socket operations (may block tasks)
- WiFi credentials not persisted to NVS by default
- No OTA update mechanism included

## Future Improvements

- Add TLS (mbedTLS) for HTTPS
- Add WebSocket for real-time updates
- Persist WiFi credentials in NVS
- Add OTA firmware update support
- Add MQTT support
- Convert HTTP client to non-blocking / asynchronous
- Improve web UI (e.g., Chart.js for visualization)

---

## Learning Outcomes

By implementing this project you learn:

- Embedded TCP/IP programming and HTTP basics
- Using esp_http_server and BSD sockets on ESP-IDF
- Cloud integration with ThingSpeak
- Peripheral drivers: RMT (WS2812B) and LEDC (PWM)
- Managing tasks with FreeRTOS
- Designing modular embedded software
