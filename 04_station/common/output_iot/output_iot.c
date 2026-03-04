#include <stdio.h>
#include <esp_log.h>
#include "driver/gpio.h"
#include "output_iot.h"

// Function to configure a GPIO pin as output
void output_io_create(gpio_num_t gpio_num)
{
    // Reset the GPIO pin to its default state before configuration
    gpio_reset_pin(gpio_num);

    /* Set the GPIO as an input/output pin (push-pull output mode) */
    gpio_set_direction(gpio_num, GPIO_MODE_INPUT_OUTPUT);
}

// Function to set the output level of a GPIO pin
void output_io_set_level(gpio_num_t gpio_num, int level)
{
    // Write the specified logic level (0 or 1) to the GPIO pin
    gpio_set_level(gpio_num, level);
}

// Function to toggle the current output level of a GPIO pin
void output_io_toggle(gpio_num_t gpio_num)
{
    // Read the current level of the GPIO pin
    int old_level = gpio_get_level(gpio_num);

    // Set the GPIO to the opposite logic level
    gpio_set_level(gpio_num, 1 - old_level);
}
