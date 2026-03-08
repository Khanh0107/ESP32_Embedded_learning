/* Non-Volatile Storage (NVS) Read and Write a Value - Example

   For other examples please check:
   https://github.com/espressif/esp-idf/tree/master/examples

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"

#define USER_NAMESPACE "user"

static nvs_handle_t my_handle;

void app_nvs_set_value(char *key, int value)
{
    int err;
    err = nvs_open(USER_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        printf("nvs open failed!\n");
    }
    else {
        printf("nvs open done\n");
    }
    err = nvs_set_i32(my_handle, key, value);
    if (err != ESP_OK) {
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
    }
    err = nvs_commit(my_handle);
    if (err != ESP_OK) {
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
    }
    nvs_close(my_handle);
}

void app_nvs_set_string(char *key, char* str)
{
    int err;
    err = nvs_open(USER_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        printf("nvs open failed!\n");
    }
    else {
        printf("nvs open done\n");
    }
    err = nvs_set_str(my_handle, key, str);
    if (err != ESP_OK) {
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
    }
    err = nvs_commit(my_handle);
    if (err != ESP_OK) {
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
    }
    nvs_close(my_handle);
}

void app_nvs_get_value(char *key, int *value)
{
    int err;
    err = nvs_open(USER_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        printf("nvs open fail!\n");
    }
    err = nvs_get_i32(my_handle, key, value);
    switch (err) {
        case ESP_OK:
            printf("Done\n");
            printf("%s = %d\n", key, *value);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            printf("The value is not initialized yet!\n");
            break;
        default :
            printf("Error (%s) reading!\n", esp_err_to_name(err));
    }
    nvs_close(my_handle);
}

void app_nvs_get_str(char *key, char *out)
{
    int err;
    size_t length = 0;
    err = nvs_open(USER_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        printf("nvs open fail\n");
    }
    err = nvs_get_str(my_handle, key, out, &length);
    err = 0;
    err = nvs_get_str(my_handle, key, out, &length);
    switch (err) {
        case ESP_OK:
            printf("Done\n");
            printf("%s = %s, length = %d\n", key, out, length);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            printf("The value is not initialized yet!\n");
            break;
        default :
            printf("Error (%s) reading!\n", esp_err_to_name(err));
    }
    nvs_close(my_handle);
}