# FreeRTOS Event-Driven Example on ESP32

## 1. Overview

This example demonstrates an event-driven architecture on the ESP32 using:

- GPIO Interrupt (Button)
- FreeRTOS Task
- Event Groups
- Software Timers
- Modular GPIO driver (input_iot / output_iot)

The system toggles an LED when a button is pressed and shows how to use FreeRTOS synchronization mechanisms properly.

## 2. System Architecture

The application is built around an Event Group that connects:

- Interrupt Service Routine (ISR)
- FreeRTOS Task

The main logic is:

```
Button Press (GPIO0)
        ↓
GPIO Interrupt
        ↓
button_callback() (ISR context)
        ↓
Set Event Bit in Event Group
        ↓
vTask1 unblocks
        ↓
Toggle LED (GPIO2)
```

## 3. Main Components

### 3.1. Event Group

```c
EventGroupHandle_t xEventGroup;
```

Event Groups are used to synchronize events between:

- ISR context
- Task context

#### Event Bits

```c
#define BIT_EVENT_BUTTON_PRESS (1 << 0)
#define BIT_EVENT_UART_RECV    (1 << 1)
```

Each bit represents a specific event in the system.

### 3.2. vTask1 – Event Handling Task

```c
uxBits = xEventGroupWaitBits(...);
```

This task:

- Blocks while waiting for events
- Wakes up when a bit is set
- Handles events safely outside ISR

If button event occurs:

```c
output_io_toggle(2);
```

The LED on GPIO2 is toggled.

### 3.3. Button Interrupt (ISR)

```c
xEventGroupSetBitsFromISR(...)
```

When GPIO0 detects a falling edge:

- ISR runs button_callback
- ISR sets the event bit
- Task wakes up and processes it

#### Important Design Principle

ISR should:

- Be short
- Not contain heavy logic
- Only signal events to tasks

This is best practice in RTOS-based systems.

### 3.4. Software Timers

Two timers are created:

- `xTimers[0]` → 500ms → Toggle LED
- `xTimers[1]` → 1000ms → Print "Hello"

Timer callback:

```c
void vTimerCallback(TimerHandle_t xTimer)
```

The timer ID determines which action to execute.

> ⚠️ **Note:** Currently the timers are not started because `xTimerStart(...)` is commented out.

## 4. Execution Flow

### 4.1 System Startup

```
app_main()
    ↓
Create timers
Configure LED GPIO2 as output
Configure GPIO0 as input with interrupt
Register button callback
Create Event Group
Create vTask1
```

## 5. Runtime Behavior

### Case 1: Button Pressed

```
User presses button
        ↓
GPIO interrupt triggered
        ↓
button_callback() runs (ISR)
        ↓
Set BIT_EVENT_BUTTON_PRESS
        ↓
vTask1 wakes up
        ↓
LED toggles
```

### Case 2: Timer Enabled (If Started)

If timers are started:

- Every 500ms → LED toggles
- Every 1000ms → "Hello" printed

## Design Pattern Used

This code follows:

**Event-Driven Architecture**

Instead of polling:

```c
while(1)
{
    if(button_pressed)
}
```

The system:

- Sleeps efficiently
- Wakes only when event occurs
- Uses RTOS synchronization primitives

## 6. Why This Design Is Important

This architecture:

- Avoids CPU waste
- Separates ISR and application logic
- Improves scalability
- Is production-ready style

This is the correct approach for:

- Embedded systems with RTOS
- Multi-tasking firmware
- Industrial-grade firmware design

## 7. Key Concepts Demonstrated

| Feature | Purpose |
|---------|---------|
| GPIO Interrupt | Detect external event |
| Event Group | Synchronize ISR and task |
| FreeRTOS Task | Process events |
| Software Timer | Execute periodic actions |
| Modular drivers | Clean architecture |

## 8. Summary

This example demonstrates how to build a structured, event-driven FreeRTOS application on ESP32 using:

- GPIO interrupt
- Event Group synchronization
- Task-based processing
- Software timers

It is a practical foundation for building scalable embedded systems.