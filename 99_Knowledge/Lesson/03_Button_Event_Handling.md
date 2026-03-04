# ESP32 Button Event Handling - Code Flow Description

## 1. System Overview

This project implements a button handling system on ESP32 using:

- GPIO Interrupt (ISR)
- FreeRTOS Software Timer (timeout detection)
- FreeRTOS Event Group (event communication)
- FreeRTOS Task (event processing)

The system detects:
- Short press
- Normal press
- Long press
- Timeout press (button held longer than 5 seconds)

---

## 2. System Initialization Flow (app_main)

When ESP32 boots:

1. Create Event Group  
   → Used to send button press events from ISR to task.

2. Configure LED GPIO as output.

3. Configure GPIO0 as input:
   - Pull-up enabled
   - Interrupt on ANY edge (press + release)

4. Register callbacks:
   - `input_event_callback` → handles press duration
   - `timeout_button_callback` → handles timeout event

5. Create FreeRTOS task `vTaskCode`  
   → Task waits for button events.

After initialization:  
`app_main()` returns and the FreeRTOS scheduler runs tasks.

---

## 3. Button Press Flow (Interrupt Based)

GPIO is configured with `GPIO_INTR_ANYEDGE`, so ISR triggers on:

- Falling edge → button pressed (LOW)
- Rising edge → button released (HIGH)

---

### 3.1 When Button is Pressed (LOW)

ISR: `gpio_input_handler()`

1. Get current tick count.
2. Save `_start = current_tick`.
3. Start FreeRTOS timer (5 seconds).

System begins counting how long the button is held.

---

### 3.2 If Button is Released Before 5 Seconds

ISR runs again:

1. Stop timer.
2. Save `_stop = current_tick`.
3. Calculate press duration:

```
_pressTick = _stop - _start
```

4. Call:

```
input_callback(gpio_num, _pressTick)
```

Control transfers to `input_event_callback()`.

---

### 3.3 Press Duration Classification

Inside `input_event_callback()`:

Convert ticks to milliseconds:

```
press_ms = tick * portTICK_PERIOD_MS
```

Classification logic:

| Duration        | Event Bit |
|----------------|-----------|
| < 1000 ms      | SHORT     |
| < 3000 ms      | NORMAL    |
| > 3000 ms      | LONG      |

Event bit is set using:

```
xEventGroupSetBitsFromISR()
```

This wakes up the waiting task.

---

## 4. Timeout Flow (Hold > 5 Seconds)

If the button is held continuously for 5 seconds:

FreeRTOS Timer expires → `vTimerCallback()` executes.

Inside:

```
timeoutButton_callback(BUTTON0)
```

Which prints:

```
Button press timeout
```

Timeout happens only if:
- Button is not released
- Timer is not stopped

---

## 5. Event Processing Task Flow

Task: `vTaskCode()`

Infinite loop:

```
xEventGroupWaitBits()
```

The task blocks until one of these bits is set:

- SHORT
- NORMAL
- LONG

When triggered, it prints:

- "Button pressed short"
- "Button pressed normal"
- "Button pressed long"

Then waits again.

---

## 6. Complete Runtime Flow

```
System Boot
    ↓
app_main()
    ↓
GPIO + Timer + EventGroup setup
    ↓
Task waiting for events
    ↓
User presses button
    ↓
ISR triggered
    ↓
Start timer + store start tick
    ↓
User releases button?
    ├── YES → Stop timer → calculate duration → set event bit → Task prints result
    └── NO  → Timer expires after 5s → timeout callback executed
```

---

## 7. Architecture Summary

**ISR Layer**
- Detects press and release
- Measures duration
- Starts/stops timer

**Timer Layer**
- Detects long hold (timeout)

**Application Layer**
- Classifies short/normal/long press
- Sends event via EventGroup

**Task Layer**
- Waits for events
- Handles business logic

---

## 8. Design Characteristics

- Minimal logic inside ISR
- Event-driven architecture
- Proper ISR-to-task communication
- FreeRTOS EventGroup synchronization
- Timer-based timeout detection

---

## 9. Behavioral Summary

| User Action                      | System Result        |
|----------------------------------|----------------------|
| Tap quickly (<1s)                | Short press          |
| Hold 1–3 seconds                 | Normal press         |
| Hold 3–5 seconds                 | Long press           |
| Hold >5 seconds (no release)     | Timeout event        |

---

## 10. Code reference
- [Code reference](https://github.com/Khanh0107/ESP32_Embedded_learning/tree/main/02_blink)


# End of Document
