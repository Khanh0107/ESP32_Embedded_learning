# ESP32 HTTP Communication with ThingSpeak

## 1. Project Overview

This project demonstrates how an ESP32 sends and receives data from
ThingSpeak Cloud using HTTP over TCP.

The ESP32:

-   Connects to WiFi\
-   Resolves the server domain name using DNS\
-   Opens a TCP socket\
-   Sends a raw HTTP request\
-   Receives and prints the HTTP response

The implementation uses:

-   ESP-IDF framework\
-   BSD sockets API\
-   FreeRTOS task

------------------------------------------------------------------------

## 2. System Architecture

```
ESP32 ↔ WiFi (802.11) ↔ Internet ↔ TCP/IP ↔ api.thingspeak.com
```

### Protocol Stack (OSI Model)

| Layer | Protocol |
|-------|----------|
| Application Layer | HTTP |
| Transport Layer | TCP |
| Network Layer | IP |
| Data Link Layer | WiFi (802.11) |

------------------------------------------------------------------------

## 3. Application Flow

### app_main()

``` c
void app_main(void)
```

Execution steps:

Initialize NVS:

``` c
nvs_flash_init();
```

Initialize TCP/IP stack:

``` c
esp_netif_init();
```

Create default event loop:

``` c
esp_event_loop_create_default();
```

Connect to WiFi:

``` c
example_connect();
```

Create HTTP task:

``` c
xTaskCreate(http_get_task, "http_get_task", 8192, NULL, 5, NULL);
```

------------------------------------------------------------------------

## 4. HTTP Task Implementation

### Step 1: DNS Resolution

``` c
getaddrinfo(WEB_SERVER, WEB_PORT, &hints, &res);
```

Converts domain name (api.thingspeak.com) to IP address.

Uses TCP (SOCK_STREAM).

------------------------------------------------------------------------

### Step 2: Create TCP Socket

``` c
sock = socket(res->ai_family, res->ai_socktype, 0);
```

Creates a TCP socket.

------------------------------------------------------------------------

### Step 3: Establish TCP Connection

``` c
connect(sock, res->ai_addr, res->ai_addrlen);
```

Performs TCP 3-way handshake:

-   SYN\
-   SYN-ACK\
-   ACK

------------------------------------------------------------------------

### Step 4: Send HTTP Request

Example GET request:

``` c
char request[] =
"GET /channels/3274847/feeds.json?api_key=YOUR_KEY&results=2 HTTP/1.1\r\n"
"Host: api.thingspeak.com\r\n"
"User-Agent: esp-idf/1.0 esp32\r\n"
"Connection: close\r\n"
"\r\n";

write(sock, request, strlen(request));
```

Key points:

-   Must end headers with `\r\n\r\n`\
-   Host header is mandatory for HTTP/1.1\
-   `Connection: close` tells server to close TCP after response

<p align="center">
    <img src="..\Image\thingspeak.png" width="600">
</p>

------------------------------------------------------------------------

### Step 5: Receive HTTP Response

``` c
do {
    len = read(sock, buffer, sizeof(buffer) - 1);
    buffer[len] = 0;
    printf("%s", buffer);
} while (len > 0);
```

Server response structure:

    HTTP/1.1 200 OK
    Content-Type: application/json
    Content-Length: xxx

    { JSON DATA }

------------------------------------------------------------------------

### Step 6: Close Socket

``` c
close(sock);
```

Releases TCP connection.

------------------------------------------------------------------------

## 5. HTTP POST Example (Upload Data)

To update ThingSpeak field:

    POST /update.json HTTP/1.1
    Host: api.thingspeak.com
    Content-Type: application/x-www-form-urlencoded
    Content-Length: 32

    api_key=YOUR_KEY&field1=50

Body format:

    key=value&key=value

------------------------------------------------------------------------

## 6. Important Technical Concepts

### 1. HTTP is Text-Based

Requests and responses are ASCII text.

### 2. Stateless Protocol

Each request is independent.

### 3. TCP Reliability

-   Ordered delivery\
-   Error checking\
-   Retransmission

### 4. Blocking Socket Behavior

`read()` blocks until:

-   Data received\
-   Server closes connection

------------------------------------------------------------------------

## 7. Memory and Task Considerations

-   Stack size: 8192 bytes recommended\
-   Use FreeRTOS task for networking\
-   Avoid blocking in main task\
-   Close socket after each request

## 8. Code reference 

- [Code reference](https://github.com/Khanh0107/ESP32_Embedded_learning/tree/main/03_http_request)
