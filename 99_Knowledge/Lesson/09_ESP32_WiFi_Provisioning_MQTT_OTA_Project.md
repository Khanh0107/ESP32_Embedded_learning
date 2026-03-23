# ESP32 WiFi Provisioning with Access Point, SmartConfig, MQTT and OTA

## 1. Project Overview

This project demonstrates a complete IoT firmware solution for the ESP32 microcontroller with the following key features:

- **WiFi Provisioning** using Access Point Mode and Station Mode (SmartConfig)
- **MQTT Communication** for remote device control
- **Over-the-Air (OTA) Updates** via HTTP for remote firmware updates
- **GPIO Control** for device operations (e.g., LED control)

### Workflow Summary

The firmware enables the ESP32 to:
1. Be configured with WiFi credentials through two methods
2. Connect to an MQTT broker for receiving remote commands
3. Be updated remotely without physical access
4. Control hardware based on MQTT messages

---

## 2. System Architecture

The system consists of three main components:

```
┌──────────────────┐
│  MQTT Client     │
│  (MQTT.fx)       │
└────────┬─────────┘
         │
         │ MQTT Publish
         │ Commands
         ▼
┌──────────────────┐       ┌──────────────────────┐
│     ESP32        │──────▶│ HTTP Firmware Server │
│                  │       │  (192.168.0.107)    │
│ • WiFi Prov.     │       └──────────────────────┘
│ • MQTT Client    │
│ • OTA Update     │
│ • GPIO Control   │
└──────────────────┘
```

---

## 3. Main Features

### 3.1 WiFi Provisioning

The system supports two WiFi provisioning methods for initial setup:

#### **Access Point (AP) Mode**

The ESP32 acts as a WiFi access point:

```
Network SSID:     Naruto
Password:         12345678
Gateway IP:       192.168.4.1
```

**Process:**
1. User connects to the ESP32's WiFi network
2. Opens web browser to `192.168.4.1/get`
3. Submits router WiFi credentials
4. ESP32 saves credentials and connects to the router

#### **SmartConfig (ESP-Touch)**

The ESP32 receives WiFi credentials from a mobile application:

**Process:**
1. Download ESP-Touch app on mobile phone
2. App sends SSID and password encoded in WiFi packet lengths
3. ESP32 decodes packets and extracts credentials
4. Automatically connects to the target WiFi network

---

### 3.2 MQTT Communication

#### Broker Configuration

```
Broker Address:   mqtt://test.mosquitto.org:1883
Port:             1883
Protocol:         MQTT v3.1.1
```

#### Subscribed Topics

```
/khanhmessi/dht11
```

#### MQTT Commands

| Command | Function | GPIO State |
|---------|----------|------------|
| `ON`    | Turn LED ON  | GPIO2 = HIGH |
| `OFF`   | Turn LED OFF | GPIO2 = LOW  |
| `SW1`   | Start OTA firmware update | N/A |

**Example:**
- Topic: `/khanhmessi/dht11`
- Payload: `ON`
- Effect: GPIO2 turns HIGH, LED turns ON

---

### 3.3 GPIO Control

**LED Control:**
- **GPIO Pin:** GPIO2
- **ON Command:** Sets GPIO2 to HIGH (3.3V)
- **OFF Command:** Sets GPIO2 to LOW (0V)

---

### 3.4 OTA Firmware Update

Over-the-Air (OTA) allows remote firmware updates without physical access to the device.

#### OTA Server Configuration

```
Firmware URL:     http://192.168.0.107/app_ota.bin
Update Trigger:   MQTT command "SW1"
File Format:      Binary firmware (.bin)
```

#### OTA Process Flow

```
MQTT "SW1" Received
        │
        ▼
OTA Start (HTTP GET)
        │
        ▼
Download Firmware Binary
        │
        ▼
Validate Firmware Checksum
        │
        ▼
Flash to OTA Partition
        │
        ▼
Update Boot Partition
        │
        ▼
Restart ESP32
```

#### OTA Partition Requirements

The flash memory must be partitioned correctly for OTA:

```
Partition Name    Type    Subtype    Offset      Size
─────────────────────────────────────────────────────
factory           app     factory    0x10000     1M
ota_0             app     ota_0      0x110000    1M
ota_1             app     ota_1      0x210000    1M
```

**Explanation:**
- `factory`: Default firmware to boot from
- `ota_0` and `ota_1`: Two alternating OTA partitions
- ESP32 alternates between them during OTA updates
- If update fails, it can roll back to previous partition

---

## 4. Project File Structure

```
project/
├── main/
│   ├── app_main.c              # Main application entry point
│   ├── app_config.c            # WiFi and system configuration
│   ├── app_mqtt.c              # MQTT client implementation
│   ├── app_ota.c               # OTA update functionality
│   ├── app_http_server.c       # HTTP server for AP provisioning
│   ├── app_http_server.h
│   ├── app_mqtt.h
│   ├── app_ota.h
│   └── CMakeLists.txt
│
├── components/                 # Custom ESP-IDF components
│
├── partition_table/            # Custom partition table
│   └── partitions.csv
│
├── CMakeLists.txt             # Project CMake configuration
└── README.md
```

---

## 5. Software Tools and Dependencies

### Required Tools

| Tool | Purpose | Version |
|------|---------|---------|
| **ESP-IDF** | Development framework | v4.2+ recommended |
| **Python 3** | Build system support | 3.6+ |
| **CMake** | Build configuration | 3.5+ |
| **GCC Cross-compiler** | C/C++ compilation for ARM | Included in ESP-IDF |

### Optional Tools

| Tool | Purpose |
|------|---------|
| **MQTT.fx** | MQTT client for testing commands |
| **Mosquitto** | MQTT broker for testing |
| **esptool.py** | Firmware flashing utility |
| **HTTP Server** | File serving for OTA firmware |

---

## 6. Configuration and Testing

### 6.1 WiFi Provisioning Testing

#### Access Point Method

1. **Flash the firmware:**
   ```bash
   idf.py erase_flash
   idf.py -p COM8 flash monitor
   ```

2. **Connect to ESP32 network:**
   - SSID: `Naruto`
   - Password: `12345678`

3. **Open web browser:**
   - Navigate to `192.168.4.1/get`
   - Enter your router WiFi credentials
   - Submit the form

4. **Verify connection:**
   - Monitor serial output for IP assignment
   - Check MQTT connection status

#### SmartConfig Method

1. **Flash firmware and start monitoring**

2. **On smartphone:**
   - Download ESP-Touch or similar app
   - Select target WiFi network
   - Enter password
   - Confirm to send

3. **ESP32 receives and connects:**
   - Listen for incoming credentials
   - Auto-connect to network
   - Display IP address on serial monitor

---

### 6.2 MQTT Testing

**Using MQTT.fx or similar client:**

#### Test 1: Turn LED ON
```
Broker:   mqtt://test.mosquitto.org:1883
Topic:    /khanhmessi/dht11
Payload:  ON
Expected: GPIO2 = HIGH (LED turns on)
```

#### Test 2: Turn LED OFF
```
Topic:    /khanhmessi/dht11
Payload:  OFF
Expected: GPIO2 = LOW (LED turns off)
```

#### Test 3: Start OTA Update
```
Topic:    /khanhmessi/dht11
Payload:  SW1
Expected: OTA process starts, ESP32 restarts
```

---

### 6.3 OTA Update Testing

**Prerequisites:**
- HTTP server running on your PC
- Firmware binary (`app_ota.bin`) accessible at configured URL
- OTA partition configured in partition table

**Steps:**
1. Compile firmware:
   ```bash
   idf.py build
   ```

2. Copy binary to HTTP server:
   ```bash
   cp C:/Apache24/htdocs/app_ota.bin
   ```

3. Send MQTT command:
   - Topic: `/khanhmessi/dht11`
   - Payload: `SW1`

4. Monitor serial output during OTA:
   - Download progress
   - Checksum validation
   - Flash process
   - Restart

---

## 7. Key Features Summary

### WiFi Provisioning
✓ Two provisioning methods (AP + SmartConfig)  
✓ NVS storage for credentials  
✓ Auto-reconnect on boot  
✓ Easy initial setup without hardcoding  

### MQTT Communication
✓ Remote device control  
✓ Command parsing  
✓ Real-time response  
✓ Scalable messaging system  

### OTA Firmware Updates
✓ Remote firmware updates  
✓ No physical access needed  
✓ Dual partition support  
✓ Checksum validation  
✓ Automatic restart  

### GPIO Control
✓ Hardware control via MQTT  
✓ LED/relay switching  
✓ Real-world IoT applications  

---

## 8. Common Applications

This architecture is used in real-world IoT products:

### Smart Home Applications
- **Smart Switches:** MQTT-controlled LED/relay
- **Smart Sensors:** WiFi environment monitoring
- **Smart Lights:** Color/brightness control over MQTT

### Industrial IoT
- **Remote Monitoring:** Sensor data publication
- **Firmware Updates:** OTA deployment to multiple devices
- **Configuration:** Easy device provisioning in field

### Consumer Products
- **Smart Plugs:** Remote power control
- **WiFi Retransmitters:** Mesh network nodes
- **Home Automation:** Integration with popular platforms

---

## 9. Conclusion

This project demonstrates a production-ready IoT firmware architecture with:

- **Flexible Provisioning:** Two methods for different deployment scenarios
- **Remote Control:** MQTT-based command system
- **Remote Updates:** OTA system for continuous improvements
- **Scalability:** Can be expanded with more GPIO and sensors

By combining these features, the ESP32 becomes a versatile IoT device suitable for both prototyping and production deployment.

---