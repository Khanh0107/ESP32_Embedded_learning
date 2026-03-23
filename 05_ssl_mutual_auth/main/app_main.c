/* MQTT Mutual Authentication Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"
#include "app_config.h"
#include "app_mqtt.h"
#include "app_nvs.h"
#include "app_ota.h"
#include <driver/gpio.h>

static const char *TAG = "Sonoff";

#define KEY "restart_cnt"
#define KEY1 "string"

static void event_handler(void* arg, esp_event_base_t event_base, 
                                int32_t event_id, void* event_data)
{
    if (event_base == MQTT_DEV_EVENT) 
    {
        if (event_id == MQTT_DEV_EVENT_CONNECTED) 
        {
            ESP_LOGW(TAG, "MQTT_DEV_EVENT_CONNECTED");
            app_mqtt_subscriber("/khanhmessi/dht11");
        } 
        else if (event_id == MQTT_DEV_EVENT_DISCONNECT) 
        {
            ESP_LOGW(TAG, "MQTT_DEV_EVENT_DISCONNECT");
        } 
        else if (event_id == MQTT_DEV_EVENT_DATA) 
        {
            ESP_LOGW(TAG, "MQTT_DEV_EVENT_DATA");
        } 
        else if (event_id == MQTT_DEV_EVENT_SUBSCRIBED) 
        {
            ESP_LOGW(TAG, "MQTT_DEV_EVENT_SUBSCRIBED");
        } 
    }   
}

void mqtt_data_callback(char* data, int len)
{
    char buf[64] = {0};

    memcpy(buf, data, len);

    printf("MQTT DATA: %s\n", buf);

    if (strstr(buf, "ON"))
    {
        gpio_set_level(2, 1);
    }
    else if (strstr(buf, "OFF"))
    {
        gpio_set_level(2, 0);
    }
    else if (strstr(buf, "SW1"))
    {
        printf("Start OTA\n");
        app_ota_start();
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK( esp_event_handler_register(MQTT_DEV_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL) );

    // Reset the GPIO pin to its default state before configuration
    gpio_reset_pin(2);

    /* Set the GPIO as an input/output pin (push-pull output mode) */
    gpio_set_direction(2, GPIO_MODE_INPUT_OUTPUT);

    // test api for nvs
    // int restart_cnt = 0;
    // app_nvs_get_value(KEY, &restart_cnt);
    // restart_cnt++;
    // app_nvs_set_value(KEY, restart_cnt);

    // char mang_set[50] = "";
    // sprintf(mang_set, "Hello World = %d", restart_cnt);

    // char mang[50];
    // app_nvs_get_str(KEY1, mang);
    // app_nvs_set_string(KEY1, mang_set);

    // json_gen_test_result_t result;
    // json_gen_test(&result, "onoff", true, "id", 1234, "str" , "Khanh");

    app_mqtt_set_data_callback(mqtt_data_callback);

    app_config();

    mqtt_app_start();
    // Sure Wifi connected before going to the next step
    // app_ota_start();
}
