/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "output_iot.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "input_iot.h"

/* An array to hold handles to the created software timers */
TimerHandle_t xTimers[2];

/* Handle for the FreeRTOS event group */
EventGroupHandle_t xEventGroup;

/* Bit definitions for different events */
#define BIT_EVENT_BUTTON_PRESS      ( 1 << 0 )   // Bit 0: Button press event
#define BIT_EVENT_UART_RECV         ( 1 << 1 )   // Bit 1: UART receive event

/* Task function that waits for events and handles them */
void vTask1(void * pvParameters )
{
    for(;;)
    {
        EventBits_t uxBits;

        /* Wait for either button press event or UART receive event */
        uxBits = xEventGroupWaitBits(
               xEventGroup,                           /* Event group to monitor */
               BIT_EVENT_BUTTON_PRESS | BIT_EVENT_UART_RECV, /* Bits to wait for */
               pdTRUE,                                /* Clear bits after exit */
               pdFALSE,                               /* Do not require both bits */
               portMAX_DELAY );                       /* Wait indefinitely */

        /* Check if button press event occurred */
        if (uxBits & BIT_EVENT_BUTTON_PRESS) {
            printf("Button pressed\n");

            /* Toggle LED on GPIO 2 when button is pressed */
            output_io_toggle(2);
        }

        /* Check if UART receive event occurred */
        if (uxBits & BIT_EVENT_UART_RECV) {
            printf("UART data received\n");
        }
    }
}

/* Callback function executed when a software timer expires */
void vTimerCallback(TimerHandle_t xTimer)
{
    /* Ensure timer handle is valid */
    configASSERT(xTimer);

    /* Retrieve timer ID to determine which timer triggered */
    int ulCount = (uint32_t)pvTimerGetTimerID(xTimer);

    /* Timer ID 0: Toggle LED */
    if (ulCount == 0) {
        output_io_toggle(2);
    }
    /* Timer ID 1: Print message */
    else if (ulCount == 1) {
        printf("Hello\n");
    }
}

/*
void vTask2(void * pvParameters )
{
    for(;;)
    {
        printf("Task 2 is running\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void vTask3(void * pvParameters )
{
    for(;;)
    {
        printf("Task 3 is running\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
*/

/* Interrupt callback triggered when button is pressed */
void button_callback(int pin)
{
    /* Only handle events from GPIO 0 (button pin) */
    if (pin == GPIO_NUM_0) {

        BaseType_t xHigherPriorityTaskWoken;

        /* Set button press event bit from ISR context */
        xEventGroupSetBitsFromISR(
            xEventGroup,
            BIT_EVENT_BUTTON_PRESS,
            &xHigherPriorityTaskWoken
        );
    }
}

/* Main application entry point */
void app_main(void)
{
    /* Create two software timers */

    /* Timer 0: toggles LED every 500ms */
    xTimers[0] = xTimerCreate(
        "TimerBlink",          /* Timer name */
        pdMS_TO_TICKS(500),    /* Timer period */
        pdTRUE,                /* Auto-reload */
        (void *)0,             /* Timer ID */
        vTimerCallback         /* Callback function */
    );

    /* Timer 1: prints "Hello" every 1000ms */
    xTimers[1] = xTimerCreate(
        "TimerPrint",
        pdMS_TO_TICKS(1000),
        pdTRUE,
        (void *)1,
        vTimerCallback
    );

    /* Configure GPIO 2 as output for LED */
    output_io_create(GPIO_NUM_2);

    /* Configure GPIO 0 as input with interrupt on falling edge */
    input_io_create(GPIO_NUM_0, HI_TO_LO);

    /* Register the button interrupt callback */
    input_set_callback(button_callback);

    /* Timers are currently disabled (commented out) */
    // xTimerStart(xTimers[0], 0);
    // xTimerStart(xTimers[1], 0);

    /* Create the event group before using it */
    xEventGroup = xEventGroupCreate();

    /* Create the main event-handling task */
    xTaskCreate(
        vTask1,          /* Task function */
        "vTask1",        /* Task name */
        1024,            /* Stack size in words */
        NULL,            /* Task parameter */
        4,               /* Task priority */
        NULL             /* Task handle */
    );

    /*
    xTaskCreate(
        vTask2,
        "vTask2",
        1024,
        NULL,
        5,
        NULL
    );

    xTaskCreate(
        vTask3,
        "vTask3",
        1024,
        NULL,
        6,
        NULL
    );
    */
}
