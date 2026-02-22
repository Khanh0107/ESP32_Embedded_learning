/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "input_iot.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "output_iot.h"

#define BLINK_GPIO 2
static EventGroupHandle_t xCreatedEventGroup;

#define BIT_EVENT_PRESS_SHORT      ( 1 << 0 )   // Bit 0: Button press event
#define BIT_EVENT_PRESS_NORMAL     ( 1 << 1 )   // Bit 1: Button press event
#define BIT_EVENT_PRESS_LONG       ( 1 << 2 )   // Bit 2: Button press event

void input_event_callback(int pin, uint64_t tick)
{
    if (pin == GPIO_NUM_0)
    {
        int press_ms = tick * portTICK_PERIOD_MS;
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;

        if (press_ms < 1000)
        {
            // press short
            xEventGroupSetBitsFromISR(xCreatedEventGroup, BIT_EVENT_PRESS_SHORT, &xHigherPriorityTaskWoken);
        }
        else if (press_ms < 3000)
        {
            // press normal
            xEventGroupSetBitsFromISR(xCreatedEventGroup, BIT_EVENT_PRESS_NORMAL, &xHigherPriorityTaskWoken);
        }
        else if (press_ms > 3000)
        {
            // press long
            xEventGroupSetBitsFromISR(xCreatedEventGroup, BIT_EVENT_PRESS_LONG, &xHigherPriorityTaskWoken);
        }
    }
}

void timeout_button_callback(int pin)
{
    if (pin == GPIO_NUM_0)
    {
        printf("Button press timeout\n");
    }
}

void vTaskCode(void * pvParameters )
{
    for(;;)
    {
        EventBits_t uxBits;

        /* Wait for either button press event or UART receive event */
        uxBits = xEventGroupWaitBits(
               xCreatedEventGroup,                           /* Event group to monitor */
               BIT_EVENT_PRESS_SHORT | BIT_EVENT_PRESS_NORMAL | BIT_EVENT_PRESS_LONG, /* Bits to wait for */
               pdTRUE,                                /* Clear bits after exit */
               pdFALSE,                               /* Do not require both bits */
               portMAX_DELAY );                       /* Wait indefinitely */

        /* Check if button press event occurred */
        if (uxBits & BIT_EVENT_PRESS_SHORT) {
            printf("Button pressed short\n");
        }

        /* Check if button press event occurred */
        if (uxBits & BIT_EVENT_PRESS_NORMAL) {
            printf("Button pressed normal\n");
        }

        /* Check if button press event occurred */
        if (uxBits & BIT_EVENT_PRESS_LONG) {
            printf("Button pressed long\n");
        }
    }
}

void app_main(void)
{
    // Configure the LED GPIO as output
    xCreatedEventGroup = xEventGroupCreate();
    output_io_create(BLINK_GPIO);

    // Configure GPIO_NUM_0 as input with interrupt on falling edge (HI_TO_LO)
    input_io_create(GPIO_NUM_0, GPIO_INTR_ANYEDGE);

    // Register the callback function to be called on input interrupt
    input_set_callback(input_event_callback);
    input_set_timeout_button(timeout_button_callback);
    /* Create the main event-handling task */
    xTaskCreate(
        vTaskCode,          /* Task function */
        "vTaskCode",        /* Task name */
        2048,            /* Stack size in words */
        NULL,            /* Task parameter */
        4,               /* Task priority */
        NULL             /* Task handle */
    );
}
