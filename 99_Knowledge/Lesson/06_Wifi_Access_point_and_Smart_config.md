# ESP32 WiFi Provisioning and Connection Notes

## 1. Overview

This module is responsible for configuring and connecting the ESP32 to a WiFi network.

The system supports **two WiFi provisioning methods**:

1. **Station Mode (SmartConfig)**
   - The ESP32 receives WiFi SSID and password from a mobile phone.

2. **Access Point Mode**
   - The ESP32 creates its own WiFi hotspot.
   - The user connects to this hotspot and configures WiFi credentials.

After configuration:

- WiFi credentials are stored in **NVS (Non-Volatile Storage)**
- The ESP32 will **automatically reconnect** after reboot.

---

## 2. WiFi Modes on ESP32

ESP32 supports multiple WiFi operating modes:

| Mode | Description |
|-----|-------------|
| WIFI_MODE_STA | ESP32 operates as a WiFi client (Station) |
| WIFI_MODE_AP | ESP32 creates a WiFi Access Point |
| WIFI_MODE_APSTA | ESP32 works as both Station and Access Point |
| WIFI_MODE_NULL | WiFi is disabled |

This project mainly uses:

- `WIFI_MODE_STA`
- `WIFI_MODE_AP`


---

## 3. WiFi Provisioning

Provisioning is the process of **providing WiFi credentials to an IoT device for the first time**.

General flow:

```
ESP32 start
      │
Check WiFi config in NVS
      │
 ┌────┴────┐
 │         │
No config  Has config
 │         │
Provision  Connect WiFi
```


---

## 4 Station Mode (SmartConfig) (ESP-Touch)

SmartConfig allows a smartphone to send:

- WiFi SSID
- WiFi Password

to the ESP32 through **WiFi broadcast packets**.

Common applications used:

- EspTouch
- ESP RainMaker
- ESP SmartConfig apps

## How Station Mode (SmartConfig) Works

The ESPTOUCH protocol transmits WiFi SSID and password to the device using the following process:

- The ESP32 enters listening mode and scans WiFi channels sequentially.
- The smartphone must be connected to the target WiFi network (encrypted).
- The mobile application sends WiFi packets with arbitrary data, but the packet lengths encode the characters of the SSID and password.
- For example, if the SSID is **mynetwork**, the ASCII value of the character **m** is **109**.  
  The application sends a packet with length **109** containing arbitrary data.  
  This process repeats for each character of the SSID and password, including CRC characters.

- The ESPTOUCH protocol may encrypt the transmitted packets, but the length pattern of the packets is preserved.

- The ESP32 detects packets with varying lengths and reconstructs the SSID and password from those lengths.

- After successfully connecting to the WiFi network, the ESP32 sends an acknowledgment to the smartphone's IP address, which was provided through the ESPTOUCH process.

- The mobile application then displays that the configuration has been completed successfully.

### Start SmartConfig

```c
esp_smartconfig_set_type(SC_TYPE_ESPTOUCH);
esp_smartconfig_start(&cfg);
```

### Important SmartConfig Events

| Event | Description |
|-------|-------------|
| SC_EVENT_SCAN_DONE | WiFi scan completed |
| SC_EVENT_FOUND_CHANNEL | Target channel found |
| SC_EVENT_GOT_SSID_PSWD | Received SSID and password |
| SC_EVENT_SEND_ACK_DONE | SmartConfig finished |

### Handling Received Credentials

When this event occurs:

`SC_EVENT_GOT_SSID_PSWD`

ESP32 performs the following steps:

- Copy SSID
- Copy Password
- Set WiFi configuration
- Connect to the router

Example:

```c
esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
esp_wifi_connect();
```

### Step by Step SmartConfig Flow

1. `idf.py erase_flash` - Erase flash memory to remove previous WiFi credentials.
2. `idf.py -p COM8 flash monitor` - Flash the new firmware and monitor the output.
3. Open the SmartConfig app on your smartphone.

<img src="..\Image\Smart_config.png">

<img src="..\Image\Smart_config_scan.png">

4. Start the SmartConfig process in the app. The ESP32 will receive the credentials and connect to the WiFi network.

<img src="..\Image\Smart_config_app_connected.png">

<img src="..\Image\Smart_config_connected.png">

## 5. Access Point Mode

### 5.1 HTTP WiFi Configuration

After the ESP32 starts in Access Point mode, an HTTP server is started to receive WiFi credentials.

Functions used:

- `app_http_server_start()`
- `app_http_server_post_set_callback()`

The HTTP server waits for WiFi credentials sent from the user.

The callback function used to process the received data:

- `http_post_data_callback()`

When credentials are received, the system triggers the event:

- `HTTP_CONFIG_DONE_BIT`

---

### 5.2 Switching from Access Point to Station Mode

After receiving WiFi credentials, the ESP32 stops the HTTP server and switches to Station mode.

Functions involved:

- `app_http_server_stop()`
- `esp_wifi_init()`
- `esp_wifi_set_mode()`
- `esp_wifi_set_config()`
- `esp_wifi_start()`
- `esp_wifi_connect()`

At this stage, the ESP32 attempts to connect to the router using the provided SSID and password.

---

### 5.3 Waiting for WiFi Connection

The system waits until the ESP32 successfully connects to the router.

Synchronization is handled using:

- `xEventGroupWaitBits()`

When the connection is successful and an IP address is assigned, the following event occurs:

- `IP_EVENT_STA_GOT_IP`

Finally, the system confirms the connection and continues execution.

### Step by Step Access Point Flow

1. `idf.py erase_flash` - Erase flash memory to remove previous WiFi credentials.
2. `idf.py -p COM8 flash monitor` - Flash the new firmware and monitor the output.
3. Connect to the WiFi network created by the ESP32 (e.g., SSID: "Naruto", Password: "12345678").
4. Open a web browser, navigate to `192.168.4.1/get` and submit the WiFi credentials for your router.

<img src="..\Image\AP_webserver_ESP.png">

<img src="..\Image\AP_ESP_receive_SSID_PASS.png">

<img src="..\Image\AP_ESP_Wifi_connected.png">

## 6. FreeRTOS, System Configuration and Logging

### 6.1 FreeRTOS Event Groups

The project uses Event Groups for task synchronization.

#### Event Bits

- `WIFI_CONNECTED_BIT`
- `ESPTOUCH_DONE_BIT`

#### Create Event Group

```c
s_wifi_event_group = xEventGroupCreate();
```

#### Set Event Bit

```c
xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
```

#### Wait for Event

```c
xEventGroupWaitBits(
    s_wifi_event_group,
    WIFI_CONNECTED_BIT,
    false,
    true,
    portMAX_DELAY
);
```

**Meaning:**

The system waits until WiFi is successfully connected before continuing execution.

### 6.2 Checking if the Device is Provisioned

**Function used:**

`is_provisioned()`

This function checks whether WiFi credentials are already stored.

**Example:**

```c
esp_wifi_get_config(WIFI_IF_STA, &wifi_config);

if (wifi_config.sta.ssid[0] != 0x00)
{
    provisioned = true;
}
```

If the SSID is not empty, the device has already been provisioned.

### 6.3 WiFi Initialization Flow

Complete initialization process:

```
app_main()
   │
   ├─ nvs_flash_init()
   │
   ├─ esp_netif_init()
   │
   ├─ esp_event_loop_create_default()
   │
   └─ app_config()
           │
           ├─ register event handlers
           │
           ├─ check provisioning status
           │
           ├─ if not provisioned
           │       ├─ SmartConfig
           │       └─ Access Point
           │
           └─ connect to WiFi
```

### 6.4 NVS (Non-Volatile Storage)

ESP32 stores WiFi credentials in NVS Flash memory.

#### Initialization:

```c
nvs_flash_init();
```

After a successful connection:

- SSID
- Password

are stored in flash.

On reboot:

The ESP32 will automatically reconnect to the saved WiFi network.

### 6.5 Logging System

ESP-IDF provides a logging system.

**Example:**

```c
ESP_LOGI(TAG, "Wifi Connected");
```

**Logging levels:**

| Level | Description |
|-------|-------------|
| ESP_LOGE | Error |
| ESP_LOGW | Warning |
| ESP_LOGI | Information |
| ESP_LOGD | Debug |
| ESP_LOGV | Verbose |

## 7. Key Knowledge from This Project

This project covers several important embedded networking concepts.

### Embedded Networking

- WiFi Station Mode
- WiFi Access Point Mode
- SmartConfig provisioning
- DHCP and IP assignment
- TCP/IP stack

### ESP-IDF Framework

- WiFi driver
- Event loop system
- NVS storage
- Logging system

### FreeRTOS

- Event Groups
- Task synchronization