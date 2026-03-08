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
#include "protocol_examples_common.h"

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

static const char *TAG = "Sonoff";

#define KEY "restart_cnt"
#define KEY1 "string"

void app_main(void)
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

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

    app_config();
    mqtt_app_start();
    // Sure Wifi connected before going to the next step
}
