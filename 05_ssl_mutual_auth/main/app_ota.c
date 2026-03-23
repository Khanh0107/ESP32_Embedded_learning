#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "string.h"

static const char *TAG = "OTA_APP";

#define OTA_URL_SIZE 256 
#define LOG_PROCESS_OTA_UPGRADE 1

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
    case HTTP_EVENT_ERROR:
        ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
        break;
    }
    return ESP_OK;
}

esp_http_client_config_t my_config = {
        .url = "http://192.168.0.107/app_ota.bin",
        .cert_pem = NULL,
        .event_handler = _http_event_handler,
        .keep_alive_enable = true,
        .skip_cert_common_name_check = true
};

int app_ota_start(void)
{
    esp_http_client_config_t *config = &my_config;

    if (!config) {
        ESP_LOGE(TAG, "esp_http_client config not found");
        return ESP_ERR_INVALID_ARG;
    }    

    esp_https_ota_config_t ota_config = {
        .http_config = config,
    };

    esp_https_ota_handle_t https_ota_handle = NULL;
    esp_err_t err = esp_https_ota_begin(&ota_config, &https_ota_handle);
    if (https_ota_handle == NULL) {
        return ESP_FAIL;
    }

    // init other object for get length
    esp_http_client_handle_t _http_client = esp_http_client_init(ota_config.http_config);
    if (_http_client == NULL)
    {
        ESP_LOGE(TAG, "Failed to initialise HTTP connection");
        goto ERROR;
    }

    err = esp_http_client_open(_http_client, 0);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        goto ERROR;
    }

    int length_image_firmware = esp_http_client_fetch_headers(_http_client);
    ESP_LOGI(TAG, "Length Image : %d", length_image_firmware);

    while (1) {
        err = esp_https_ota_perform(https_ota_handle);
        if (err != ESP_ERR_HTTPS_OTA_IN_PROGRESS) {
            break;
        }
            
        #if (LOG_PROCESS_OTA_UPGRADE == 1)
        int process_len = esp_https_ota_get_image_len_read(https_ota_handle);
        uint8_t duty = (process_len * 100 / length_image_firmware);
        ESP_LOGI(TAG, "Process OTA : %d%%", duty);
        #endif
    }

    esp_err_t ota_finish_err = esp_https_ota_finish(https_ota_handle);
    if (err != ESP_OK) {
        /* If there was an error in esp_https_ota_perform(),
           then it is given more precedence than error in esp_https_ota_finish()
         */
        return err;
    } else if (ota_finish_err != ESP_OK) {
        return ota_finish_err;
    }

    // Upgrade successful, restart the board to boot into the new firmware
    esp_restart();

ERROR:
    ESP_LOGE(TAG, "ERROR");
    return ESP_FAIL;
}
