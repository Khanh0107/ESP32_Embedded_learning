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
#include "output_iot.h"

// Macro defining which GPIO pin is used for blinking LED
// The value is taken from menuconfig (CONFIG_BLINK_GPIO)
#define BLINK_GPIO CONFIG_BLINK_GPIO

// Callback function that will be executed when an input event occurs
void input_event_callback(int pin)
{
    // Check if the interrupt came from GPIO_NUM_0 (the button pin)
    if(pin == GPIO_NUM_0)
    {
        // Toggle the LED state whenever the button is pressed
        output_io_toggle(BLINK_GPIO);
    }
}

void app_main(void)
{
    // Configure the LED GPIO as output
    output_io_create(BLINK_GPIO);

    // Configure GPIO_NUM_0 as input with interrupt on falling edge (HI_TO_LO)
    input_io_create(GPIO_NUM_0, HI_TO_LO);

    // Register the callback function to be called on input interrupt
    input_set_callback(input_event_callback);
}
