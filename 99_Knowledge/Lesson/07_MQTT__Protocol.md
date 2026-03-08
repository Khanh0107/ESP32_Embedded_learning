# 1. Introduction to MQTT

## 1.1 What is MQTT?

MQTT (Message Queuing Telemetry Transport) is a lightweight messaging protocol designed for low-bandwidth, high-latency, or unreliable networks. It is widely used in IoT (Internet of Things) systems because it allows devices with limited resources (such as microcontrollers) to communicate efficiently.

Key characteristics of MQTT:

- Lightweight protocol
- Low bandwidth usage
- Low power consumption
- Supports unreliable networks
- Works well with embedded devices
- Uses asynchronous communication

MQTT runs on top of TCP/IP, meaning it uses reliable TCP connections to transmit messages between clients and a broker.

## 2. MQTT Communication Model

MQTT follows a Publish/Subscribe communication model, which is different from the traditional Client/Server Request-Response model used by protocols like HTTP.

Instead of sending requests directly to another device, clients send messages to a broker, which distributes them to interested subscribers.

### 2.1 MQTT Architecture

<img src="..\Image\MQTT_Process.png">

In this architecture:

- Devices that send data are called Publishers
- Devices that receive data are called Subscribers
- The central server that manages communication is the Broker

## 3. Main Components of MQTT

### 3.1 MQTT Broker

The MQTT Broker is the central component of the system. It is responsible for:

- Receiving messages from publishers
- Filtering messages by topic
- Distributing messages to subscribers
- Managing client connections
- Handling authentication and authorization

Examples of popular MQTT brokers:

| Broker     | Description                     |
|------------|---------------------------------|
| Mosquitto  | Lightweight open-source broker  |
| HiveMQ     | Enterprise-grade MQTT platform  |
| EMQX       | Highly scalable MQTT broker     |
| AWS IoT Core | Cloud-managed MQTT service    |

### 3.2 Publisher

A Publisher is a client that sends messages to the broker.

For example, an ESP32 reading a temperature sensor may publish data like this:

**Topic:** `/sensor/dht11`

**Payload:**
```json
{
  "temperature": 30,
  "humidity": 65
}
```

The publisher does not need to know who receives the message.

### 3.3 Subscriber

A Subscriber is a client that registers interest in a specific topic.

Example:

**Subscribe topic:** `/sensor/dht11`

Whenever a message is published to that topic, the broker forwards it to the subscriber.

Subscribers do not know who published the message.

## 4. MQTT Topics

A Topic is a hierarchical string used to categorize messages.

Topics act like communication channels.

Examples:

- `home/livingroom/temperature`
- `home/livingroom/light`
- `factory/machine1/status`
- `device/esp32/data`

Topics can be organized in a hierarchical structure using `/`.

Example:

```
home/
  ├── livingroom/
  │      ├── temperature
  │      └── humidity
  └── kitchen/
         └── temperature
```

## 5. MQTT Message Structure

An MQTT message contains the following elements:

| Field      | Description                  |
|------------|------------------------------|
| Topic      | Destination channel          |
| Payload    | Actual data                  |
| QoS        | Quality of Service           |
| Retain Flag| Whether broker stores message|

Example message:

**Topic:** `/sensor/dht11`

**Payload:**
```json
{
  "temp": 28,
  "humidity": 70
}
```

## 6. Quality of Service (QoS)

Here are 3 QoS (Quality of Service) options when "publishing" and "subscribing":

- **QoS 0:** The Broker/Client will send data exactly once. The transmission is acknowledged only by the TCP/IP protocol, similar to a "fire and forget" approach (literally: like abandoning a child in the market).
- **QoS 1:** The Broker/Client will send data with at least one acknowledgment from the other end, meaning there may be more than one confirmation that the data has been received.
- **QoS 2:** The Broker/Client ensures that when data is sent, the receiving side receives it exactly once. This process must go through a 4-way handshake.

## 7. MQTT over TLS

MQTT connections can use:

- `mqtt://`
- `mqtts://`

The `mqtts` protocol means MQTT over TLS (Transport Layer Security).

TLS provides:

- Encryption
- Authentication
- Data integrity

Typical ports:

| Port | Protocol                     |
|------|------------------------------|
| 1883 | MQTT (unencrypted)           |
| 8883 | MQTT over TLS                |
| 8884 | MQTT TLS with mutual authentication |

## 9. MQTT Client Library in ESP-IDF

ESP-IDF provides the MQTT client library: `esp-mqtt`

Important functions include:

| Function                  | Purpose              |
|---------------------------|----------------------|
| `esp_mqtt_client_init`    | Initialize client    |
| `esp_mqtt_client_start`   | Start connection     |
| `esp_mqtt_client_publish` | Send message         |
| `esp_mqtt_client_subscribe`| Subscribe topic     |

## 10. Event-Based MQTT Handling

The ESP-IDF MQTT client uses an event-driven architecture.

Events notify the application about connection status and incoming messages.

Common events:

| Event                  | Description              |
|------------------------|--------------------------|
| `MQTT_EVENT_CONNECTED` | Connection established   |
| `MQTT_EVENT_DISCONNECTED`| Connection lost         |
| `MQTT_EVENT_SUBSCRIBED`| Subscription successful  |
| `MQTT_EVENT_PUBLISHED` | Message published        |
| `MQTT_EVENT_DATA`      | Data received            |

## 11. Code Analysis of app_mqtt.c

### 11.1 Header Files

```c
#include "mqtt_client.h"
```

This imports the ESP-IDF MQTT library.

```c
#include "json_parser.h"
#include "json_generator.h"
```

These libraries allow the application to:

- Generate JSON data
- Parse JSON messages

### 11.2 TLS Certificate Loading

```c
extern const uint8_t client_cert_pem_start[]
extern const uint8_t client_key_pem_start[]
```

These symbols reference embedded certificate files compiled into the firmware.

ESP-IDF uses the linker to include certificate binaries using:

- `_binary_client_crt_start`
- `_binary_client_key_start`

These certificates are required for TLS authentication with the MQTT broker.

### 11.3 JSON Buffer Handler

**Function:**

```c
static void flush_str(char *buf, void *priv)
```

**Purpose:**

- Append generated JSON fragments
- Store them in a result buffer

**Key operation:**

```c
memcpy(result->buf + result->offset, buf, strlen(buf));
```

This copies generated JSON pieces into the final output buffer.

### 11.4 MQTT Event Handler

The function:

```c
mqtt_event_handler()
```

handles different MQTT events.

#### 11.4.1 Connection Event

`MQTT_EVENT_CONNECTED`

When the ESP32 connects to the broker, it subscribes to a topic:

```c
esp_mqtt_client_subscribe(client, "/khanhmessi/dht11", 0);
```

#### 11.4.2 Subscription Event

`MQTT_EVENT_SUBSCRIBED`

Once subscription is confirmed, the device publishes a message:

```c
esp_mqtt_client_publish(client, "/khanhmessi/dht11", "data", 0, 0, 0);
```

#### 11.4.3 Data Reception Event

`MQTT_EVENT_DATA`

When data arrives, the topic and payload are printed:

```c
printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
printf("DATA=%.*s\r\n", event->data_len, event->data);
```

### 11.5 MQTT Client Configuration


**Function:**

```c
mqtt_app_start()
```

This function configures and starts the MQTT client.

**Configuration structure:**

```c
esp_mqtt_client_config_t mqtt_cfg
```

Important parameters:

- `.uri`
- `.event_handle`
- `.client_cert_pem`
- `.client_key_pem`

### 11.6 MQTT Client Startup

The client is initialized and started:

```c
esp_mqtt_client_init(&mqtt_cfg);
esp_mqtt_client_start(client);
```

Connection flow:

```
ESP32
  ↓
TCP connection
  ↓
TLS handshake
  ↓
MQTT CONNECT packet
  ↓
Broker responds with CONNACK
```

## 13. Program Execution Flow

The complete system flow:

```
ESP32 Boot
   ↓
Initialize NVS
   ↓
Initialize network stack
   ↓
Connect to WiFi
   ↓
Start MQTT client
   ↓
TLS handshake
   ↓
MQTT connection established
   ↓
Subscribe topic
   ↓
Publish message
   ↓
Receive data
```

## 14. Practice output

1. Setting up MQTT broker on cloud service.
<img src="..\Image\setup_MQTT.png">

<img src="..\Image\setup_topic_MQTT.png">

2. Send data from MQTTFX client and receive it on ESP32

<img src="..\Image\ESP_receive_data_from_MQTTFx.png">

### 15. Code reference

- [Code reference](../../05_ssl_mutual_auth)
