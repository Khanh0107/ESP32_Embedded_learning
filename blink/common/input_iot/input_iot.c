#include <stdio.h>
#include <esp_log.h>
#include "driver/gpio.h"
#include "input_iot.h"

// Global variable to store the user-registered callback function
input_callback_t input_callback = NULL;

// GPIO interrupt handler function
// IRAM_ATTR places this function in IRAM so it can be safely used inside an ISR
static void IRAM_ATTR gpio_input_handler(void* arg) {
    // Retrieve the GPIO number passed as argument
    int gpio_num = (uint32_t)arg;

    // Call the user callback function with the GPIO number
    input_callback(gpio_num);
}

// Function to configure a GPIO pin as input with interrupt capability
void input_io_create(gpio_num_t gpio_num, interrupt_type_edle_t type)
{
    // Reset the GPIO pin to its default state before configuration
    gpio_reset_pin(gpio_num);

    // Set the GPIO direction to input mode
    gpio_set_direction(gpio_num, GPIO_MODE_INPUT);

    // Enable internal pull-up resistor for the input pin
    gpio_set_pull_mode(gpio_num, GPIO_PULLUP_ONLY);

    // Configure the interrupt trigger type (rising edge, falling edge, etc.)
    gpio_set_intr_type(gpio_num, type);

    // Install the GPIO ISR service (required before adding any ISR handlers)
    gpio_install_isr_service(0);

    // Attach the interrupt handler function to the specified GPIO pin
    gpio_isr_handler_add(gpio_num, gpio_input_handler, (void*)gpio_num);
}

// Function to read the current logic level of a GPIO input pin
int input_io_get_level(gpio_num_t gpio_num)
{
    // Return the current state of the GPIO pin (0 or 1)
    return gpio_get_level(gpio_num);
}

// Function to register a user-defined callback for input events
void input_set_callback(void * cb)
{
    // Store the user callback function pointer in the global variable
    input_callback = cb;
}
