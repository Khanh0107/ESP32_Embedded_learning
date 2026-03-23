# OTA Firmware Update on ESP32 (ESP-IDF)

## 1. Introduction

### 1.1. Introduction to OTA

OTA (Over-The-Air Update) is a mechanism that allows embedded devices to update their firmware remotely through a network connection, without requiring physical access to the device.

For IoT devices like ESP32, OTA is extremely important because devices are often deployed in remote locations.

Typical OTA workflow:

- Developer → Build Firmware (.bin)
- Server → Host firmware file
- Device → Download firmware
- Device → Write firmware to flash
- Device → Reboot into new firmware

OTA provides:

- Remote firmware upgrade
- Bug fixing without physical access
- Feature updates
- Security patch deployment

### 1.2. Why OTA is Important in IoT

IoT devices are often:

- Installed in buildings
- Installed in factories
- Installed in remote locations

Without OTA, updating firmware would require:

- Connecting USB
- Re-flashing firmware manually
- Restarting device

With OTA:

- Internet → Device downloads firmware → Device upgrades itself

Advantages:

| Advantage | Description |
|-----------|-------------|
| Remote updates | No physical connection required |
| Scalability | Update thousands of devices |
| Security | Deploy security patches quickly |
| Maintenance | Fix bugs after deployment |

## 2. OTA Architecture and Process

### 2.1. OTA Architecture

A typical OTA system includes 3 components:

#### 1. Firmware Server

- Hosts the firmware file.
- Example: http://192.168.0.107/app_ota.bin

Server types:

- Apache
- Nginx
- Cloud storage
- AWS S3
- Custom HTTP server

#### 2. Embedded Device

Device responsibilities:

- Connect to WiFi
- Request firmware from server
- Download firmware
- Write firmware to flash
- Restart

#### 3. Bootloader

- The bootloader decides which firmware partition to boot.
- The ESP32 bootloader supports OTA by managing multiple firmware partitions.

### 2.2. ESP32 Flash Partition Layout

OTA requires multiple application partitions.

Example partition table:

```csv
# Name,   Type, SubType, Offset,  Size, Flags
# Note: if you have increased the bootloader size, make sure to update the offsets to avoid overlap,,,.
nvs,      data, nvs,     ,  0x4000,
otadata,  data, ota,     ,  0x2000,
phy_init, data, phy,     ,  0x1000,
ota_0,    app,  ota_0,   0x10000, 0x160000,
ota_1,    app,  ota_1,   0x180000,0x160000,
storage,  data, nvs,     0x2E0000,0x30000,
```

Explanation:

| Partition | Purpose |
|-----------|---------|
| Bootloader | Starts the system |
| Partition Table | Flash layout definition |
| NVS | Store WiFi config |
| OTA data | OTA state |
| OTA_0 | Firmware slot 0 |
| OTA_1 | Firmware slot 1 |

### 2.3. OTA Upgrade Process

Below is the OTA flow.

- **Initial Flash State**
  - First firmware is written to Factory or OTA_0
  - Bootloader
  - Partition Table
  - NVS
  - OTA data
  - Factory App
  - OTA_0
  - OTA_1

- **First OTA Upgrade**
  - New firmware is downloaded and written to OTA_1
  - Current running: OTA_0
  - Download → write → OTA_1
  - Bootloader flag → OTA_1

- **Second OTA Upgrade**
  - Next firmware will overwrite OTA_0
  - Current running: OTA_1
  - Download → write → OTA_0
  - Bootloader flag → OTA_0

This alternating mechanism ensures rollback safety.

<img src="..\Image\OTA_Flow.png" >

### 2.4. OTA Flow Diagram

The OTA update process can be visualized as:

1. Device connects to WiFi
2. Device requests firmware from server
3. Device downloads firmware
4. Firmware written to OTA partition
5. Bootloader flag updated
6. Device restarts

## 3. Firmware Distribution

### 3.1. Overview

Firmware Distribution involves hosting compiled firmware files on a web server so ESP32 can download them via HTTP.

**Process:**
1. Compile firmware → Generate `app_ota.bin`
2. Copy `app_ota.bin` to web server (Apache, Nginx, etc.)
3. ESP32 downloads from: `http://[server_ip]/app_ota.bin`
4. ESP32 verifies and boots new firmware

---

### 3.2. Web Server Setup (Apache)

#### **Installation**

1. Download: https://www.apachelounge.com/download/
2. Extract to: `C:\Apache24`
3. Run: `C:\Apache24\bin\httpd.exe`
4. Test: Open browser → `http://localhost`

#### **Place Firmware**

Copy firmware file to: `C:\Apache24\htdocs\app_ota.bin`

#### **Access Firmware**

Find your PC IP:
```bash
ipconfig
```

Download URL: `http://192.168.x.x/app_ota.bin`

Example: `http://192.168.1.107/app_ota.bin`

---

### 3.3. Server Options

| Server | Use Case | Pros |
|--------|----------|------|
| **Apache** | Development | Simple, local control |
| **Nginx** | Production | Lightweight, fast |
| **Cloud (S3, Azure)** | Large scale | Globally distributed, scalable |
| **Custom** | Integration | Full customization |

---

### 3.4. Best Practices

- **Version Management**: Keep multiple firmware versions (`v1.0.bin`, `v1.1.bin`, etc.)
- **HTTPS**: Use in production for security
- **Checksums**: Verify firmware integrity with SHA256
- **Backups**: Maintain redundant servers
- **Logging**: Monitor firmware downloads
- **Authentication**: Restrict access if needed

---

### 3.5. Troubleshooting

| Problem | Cause | Solution |
|---------|-------|----------|
| Cannot access server | Firewall blocking port 80 | Allow Apache in Windows Firewall |
| Download fails | File not in htdocs | Verify file location: `dir C:\Apache24\htdocs\app_ota.bin` |
| Slow download | Weak WiFi | Move ESP32 closer to router |

---

### 3.6. Summary

- **Purpose**: Host firmware files for OTA downloads
- **Setup**: Install Apache, copy .bin file to htdocs
- **Access**: Use HTTP URL from device's network
- **Security**: Use HTTPS, checksums in production
- **Verification**: Check HTTP headers (Content-Length)

## 4. ESP-IDF OTA Implementation

### 4.1. ESP-IDF OTA API

ESP-IDF provides a full OTA API.

Important APIs:

| Function | Purpose |
|----------|---------|
| `esp_https_ota_begin()` | Initialize OTA |
| `esp_https_ota_perform()` | Download firmware |
| `esp_https_ota_finish()` | Finish OTA |
| `esp_https_ota_get_image_len_read()` | Get downloaded size |
| `esp_restart()` | Restart device |

### 4.2. OTA HTTP Client Event Handler

Your code defines a HTTP event handler.

```c
esp_err_t _http_event_handler(esp_http_client_event_t *evt)
```

This function handles HTTP events during OTA.

Events include:

| Event | Description |
|-------|-------------|
| `HTTP_EVENT_ERROR` | Error occurred |
| `HTTP_EVENT_ON_CONNECTED` | HTTP connected |
| `HTTP_EVENT_HEADER_SENT` | Header sent |
| `HTTP_EVENT_ON_HEADER` | Header received |
| `HTTP_EVENT_ON_DATA` | Data received |
| `HTTP_EVENT_ON_FINISH` | Download finished |
| `HTTP_EVENT_DISCONNECTED` | Connection closed |

Example:

```c
case HTTP_EVENT_ON_DATA:
ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
```

This logs incoming firmware chunks.

### 4.3. HTTP Client Configuration

```c
esp_http_client_config_t my_config
```

Configuration parameters:

```c
.url = "http://192.168.0.107/app_ota.bin"  // Firmware download URL
.cert_pem = NULL                            // No TLS certificate (HTTP, not HTTPS)
.event_handler = _http_event_handler        // HTTP event callback
.keep_alive_enable = true                   // Persistent connection
.skip_cert_common_name_check = true         // Skip cert validation (testing only)
```

### 4.4. OTA Initialization

OTA begins with:

```c
esp_https_ota_handle_t https_ota_handle = NULL;

// Initialize OTA config
esp_https_ota_config_t ota_config = {
    .http_config = &http_config,
};

// Start OTA process
esp_err_t err = esp_https_ota_begin(&ota_config, &https_ota_handle);
```

This function:

- Connects to server
- Starts firmware download
- Prepares OTA partition

### 4.5. Getting Firmware Size

Your code manually fetches firmware size.

```c
// Initialize HTTP client
esp_http_client_handle_t _http_client = esp_http_client_init(&http_config);

// Open HTTP connection
esp_http_client_open(_http_client, 0);

// Fetch headers to get Content-Length
int length_image_firmware = esp_http_client_fetch_headers(_http_client);

// Returns: Content-Length (e.g., 154080 bytes)
ESP_LOGI(TAG, "Firmware size: %d bytes", length_image_firmware);
```

### 4.6. OTA Download Loop

Firmware download happens here:

```c
while (1)
{
    err = esp_https_ota_perform(https_ota_handle);

    if (err != ESP_ERR_HTTPS_OTA_IN_PROGRESS)
        break;
}
```

This loop:

- Downloads firmware chunk
- Writes chunk to flash
- Repeats until complete

### 4.7. OTA Progress Display

Your code calculates and displays progress during firmware download.

Progress calculation:

```c
// Get bytes downloaded so far
int process_len = esp_https_ota_get_image_len_read(https_ota_handle);

// Calculate percentage
uint8_t duty = (process_len * 100 / length_image_firmware);

// Log progress
ESP_LOGI(TAG, "OTA Progress: %d%% (%d / %d bytes)", 
         duty, process_len, length_image_firmware);
```

Example output:

```
OTA Progress: 10% (15408 / 154080 bytes)
OTA Progress: 35% (53928 / 154080 bytes)
OTA Progress: 80% (123264 / 154080 bytes)
OTA Progress: 100% (154080 / 154080 bytes)
```

### 4.8. OTA Finish

After firmware download completes:

```c
// Finalize OTA process
esp_err_t err = esp_https_ota_finish(https_ota_handle);

if (ESP_OK != err) {
    if (ESP_ERR_OTA_VALIDATE_FAILED == err) {
        ESP_LOGE(TAG, "Image validation failed, image is corrupted");
    } else {
        ESP_LOGE(TAG, "Image validation failed");
    }
} else {
    ESP_LOGI(TAG, "OTA update completed successfully");
}
```

This function:

- Verifies firmware integrity
- Updates OTA data partition
- Marks new firmware as bootable

### 4.9. Device Restart

After OTA finishes successfully, restart the device to load new firmware:

```c
// Restart the device
ESP_LOGI(TAG, "Restarting device to apply new firmware...");
esp_restart();
```

Device behavior after restart:

- Device reboots
- Bootloader checks OTA data partition
- Bootloader loads new firmware from OTA partition
- New firmware starts executing

### 4.10. Bootloader OTA Decision

The bootloader checks OTA metadata to decide which partition to boot:

```c
// Bootloader reads otadata partition
otadata_state.ota_seq = 1;  // or 0

// If ota_seq = 1, boot from OTA_1
// If ota_seq = 0, boot from OTA_0
```

Example bootloader log output:

```
boot: Set actual ota_seq=1 in otadata
boot: Mapping ota_1 (OTA1) partition offsets 0x180000, size 0x200000
boot: Loading app from partition at offset 0x180000
```

This means:

- Next boot will use OTA_1 partition
- Bootloader loads firmware from offset 0x180000
- ESP32 starts executing new firmware

## 5. Summary and Benefits

### 5.1. Summary

OTA on ESP32 works as follows:

- Device connects to WiFi
- Device downloads firmware from HTTP server
- Firmware written to inactive OTA partition
- OTA metadata updated
- Device restarts
- Bootloader runs new firmware

### 5.2. Key Benefits

OTA provides:

- Remote firmware upgrade
- Safe dual-partition update
- Automatic rollback capability
- Large-scale IoT maintenance

### 5.3. Code reference

- [Code reference](../../05_ssl_mutual_auth)