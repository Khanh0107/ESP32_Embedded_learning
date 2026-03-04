/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "output_iot.h"
#include "dht11.h"
#include "ledc_app.h"
#include "http_server_app.h"
#include "ws2812b.h"

#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "cJSON.h"

#define WEB_SERVER "api.thingspeak.com"
#define WEB_PORT   "80"
#define WEB_PATH   "/channels/3274847/feeds.json?api_key=9BP8J792O28ULOXZ&results=1"
/* The examples use WiFi configuration that you can set via project configuration menu

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_ESP_WIFI_SSID      "BO 9999"
#define EXAMPLE_ESP_WIFI_PASS      "113113113bo"
#define EXAMPLE_ESP_MAXIMUM_RETRY  CONFIG_ESP_MAXIMUM_RETRY

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;
static struct dht11_reading dht11_last_data, dht11_cur_data;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static const char *TAG = "wifi station";

static int s_retry_num = 0;

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // esp_event_handler_instance_t instance_any_id;
    // esp_event_handler_instance_t instance_got_ip;
    // ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
    //                                                     ESP_EVENT_ANY_ID,
    //                                                     &event_handler,
    //                                                     NULL,
    //                                                     NULL));
    // ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
    //                                                     IP_EVENT_STA_GOT_IP,
    //                                                     &event_handler,
    //                                                     NULL,
    //                                                     NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
            /* Setting a password implies station will connect to all security modes including WEP/WPA.
             * However these modes are deprecated and not advisable to be used. Incase your Access point
             * doesn't support WPA2, these mode can be enabled by commenting below line */
	     .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    }
}

void switch_data_callback(char* data, int len)
{
   if (*data == '1') {
        output_io_set_level(GPIO_NUM_2, 1);
   }
   else{
        output_io_set_level(GPIO_NUM_2, 0);
   }
}

void slider_data_callback(char* data, int len)
{
    char number_str[3];
    memcpy(number_str, data, 3);
    int duty = atoi(number_str);
    printf("Duty: %d\n", duty);
    LedC_Set_Duty(0, duty);
}

void rgb_data_callback(char* data, int len)
{
    if(len != 6) return;

    char r_str[3] = {data[0], data[1], 0};
    char g_str[3] = {data[2], data[3], 0};
    char b_str[3] = {data[4], data[5], 0};

    uint8_t r = strtol(r_str, NULL, 16);
    uint8_t g = strtol(g_str, NULL, 16);
    uint8_t b = strtol(b_str, NULL, 16);

    printf("RGB => R:%d G:%d B:%d\n", r, g, b);

    for(int i = 0; i < 8; i++)
    {
        WS2812b_Set_Color(i, r, g, b);
    }

    WS2812b_Update_Color();
}

void dht11_data_callback(void)
{
    char resp[100];
    sprintf(resp, "{\"temperature\": \"%.1f\",\"humidity\": \"%.1f\"}", dht11_last_data.temperature, dht11_last_data.humidity);
    dht11_response(resp, strlen(resp));
}

void wifi_data_callback(char* data, int len)
{
   // data format: ssid@password@
    char ssid[30] = "";
    char pass[30] = "";
    printf("->data: %s\n", data);
    char *pt = strtok(data, "@");
    if(pt)
        strcpy(ssid, pt);
    pt = strtok(NULL, "@");
    if(pt)
        strcpy(pass, pt);

    printf("->ssid: %s\n", ssid);
    printf("->pass: %s\n", pass);

    // stop_webserver();
    esp_wifi_disconnect();
    esp_wifi_stop();
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
            /* Setting a password implies station will connect to all security modes including WEP/WPA.
            * However these modes are deprecated and not advisable to be used. Incase your Access point
            * doesn't support WPA2, these mode can be enabled by commenting below line */
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,

            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };

    strcpy((char*)wifi_config.sta.ssid,ssid);
    strcpy((char*)wifi_config.sta.password,pass);
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    // wifi_config_t wifi_config_get;
    // esp_wifi_get_config(WIFI_IF_STA, &wifi_config_get);
    // printf("--> ssid: %s\n", wifi_config_get.sta.ssid);
    // printf("--> pass: %s\n", wifi_config_get.sta.password);
    esp_wifi_start();
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT,
            pdTRUE,
            pdFALSE,
            portMAX_DELAY);
    // start_webserver();
}

static void thingspeak_get_task(void *pvParameters)
{
    char recv_buf[1024];

    while (1) {

        struct addrinfo hints = {
            .ai_family = AF_INET,
            .ai_socktype = SOCK_STREAM,
        };

        struct addrinfo *res;

        int err = getaddrinfo(WEB_SERVER, WEB_PORT, &hints, &res);
        if (err != 0 || res == NULL) {
            ESP_LOGE("TS", "DNS lookup failed");
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            continue;
        }

        int sock = socket(res->ai_family, res->ai_socktype, 0);
        if (sock < 0) {
            freeaddrinfo(res);
            continue;
        }

        if (connect(sock, res->ai_addr, res->ai_addrlen) != 0) {
            close(sock);
            freeaddrinfo(res);
            continue;
        }

        freeaddrinfo(res);

        char request[512];
        sprintf(request,
                "GET %s HTTP/1.1\r\n"
                "Host: %s\r\n"
                "Connection: close\r\n\r\n",
                WEB_PATH, WEB_SERVER);

        write(sock, request, strlen(request));

        int len = read(sock, recv_buf, sizeof(recv_buf) - 1);
        if (len > 0) {
            recv_buf[len] = 0;

            char *json_start = strstr(recv_buf, "\r\n\r\n");
            if (json_start) {
                json_start += 4;

                cJSON *root = cJSON_Parse(json_start);
                if (root) {
                    cJSON *feeds = cJSON_GetObjectItem(root, "feeds");
                    if (cJSON_GetArraySize(feeds) > 0) {

                        cJSON *item = cJSON_GetArrayItem(feeds, 0);

                        cJSON *temp = cJSON_GetObjectItem(item, "field1");
                        cJSON *humi = cJSON_GetObjectItem(item, "field2");

                        if (temp && humi) {
                            float temperature = atof(temp->valuestring);
                            float humidity = atof(humi->valuestring);

                            ESP_LOGI("TS", "ThingSpeak Temp: %.1f", temperature);
                            ESP_LOGI("TS", "ThingSpeak Humi: %.1f", humidity);

                            // Ví dụ điều khiển LED theo nhiệt độ
                            if (temperature > 30) {
                                output_io_set_level(GPIO_NUM_2, 1);
                            } else {
                                output_io_set_level(GPIO_NUM_2, 0);
                            }
                        }
                    }
                    cJSON_Delete(root);
                }
            }
        }

        close(sock);

        vTaskDelay(10000 / portTICK_PERIOD_MS); // update 10s
    }
}

static void thingspeak_send_task(void *pvParameters)
{
    char request[512];
    char recv_buf[512];

    while (1) {

        struct addrinfo hints = {
            .ai_family = AF_INET,
            .ai_socktype = SOCK_STREAM,
        };

        struct addrinfo *res;

        if (getaddrinfo("api.thingspeak.com", "80", &hints, &res) != 0)
            continue;

        int sock = socket(res->ai_family, res->ai_socktype, 0);
        if (sock < 0) {
            freeaddrinfo(res);
            continue;
        }

        if (connect(sock, res->ai_addr, res->ai_addrlen) != 0) {
            close(sock);
            freeaddrinfo(res);
            continue;
        }

        freeaddrinfo(res);

        char post_data[128];
        sprintf(post_data,
                "api_key=9BP8J792O28ULOXZ&field1=%.1f&field2=%.1f",
                dht11_last_data.temperature,
                dht11_last_data.humidity);

        sprintf(request,
                "POST /update HTTP/1.1\r\n"
                "Host: api.thingspeak.com\r\n"
                "Connection: close\r\n"
                "Content-Type: application/x-www-form-urlencoded\r\n"
                "Content-Length: %d\r\n\r\n"
                "%s",
                strlen(post_data), post_data);

        write(sock, request, strlen(request));

        read(sock, recv_buf, sizeof(recv_buf)-1);

        close(sock);

        vTaskDelay(20000 / portTICK_PERIOD_MS); // ThingSpeak giới hạn 15s
    }
}

void app_main(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    http_set_callback_switch(switch_data_callback);
    http_set_callback_dht11(dht11_data_callback);
    http_set_callback_slider(slider_data_callback);
    http_set_callback_wifi_info(wifi_data_callback);
    http_set_rgb_callback(rgb_data_callback);

    // output_io_create(GPIO_NUM_2);
    DHT11_init(GPIO_NUM_4);
    LedC_Init();
    LedC_Add_Pin(GPIO_NUM_2, 0);

    WS2812b_Init(GPIO_NUM_5, 8);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");

    wifi_init_sta();
    start_webserver();

    xTaskCreate(thingspeak_get_task, "thingspeak_get_task", 8192, NULL, 5, NULL);
    xTaskCreate(thingspeak_send_task, "thingspeak_send_task", 4096, NULL, 5, NULL);
    
    while (1) {
        dht11_cur_data = DHT11_read();
        if(dht11_cur_data.status == 0){
            dht11_last_data = dht11_cur_data;
            // printf("Temperature: %.1f, Humidity: %.1f\n", dht11_cur_data.temperature, dht11_cur_data.humidity);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
