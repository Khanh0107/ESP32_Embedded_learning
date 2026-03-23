#ifndef __APP_MQTT_H__
#define __APP_MQTT_H__

ESP_EVENT_DECLARE_BASE(MQTT_DEV_EVENT);

typedef enum {
    MQTT_DEV_EVENT_CONNECTED,
    MQTT_DEV_EVENT_DISCONNECT,
    MQTT_DEV_EVENT_DATA,
    MQTT_DEV_EVENT_SUBSCRIBED,
    MQTT_DEV_EVENT_UNSUBSCRIBED,
    MQTT_DEV_EVENT_PUBLISHED
} mqtt_event_id_t;

typedef struct {
    char buf[256];
    int offset;
} json_gen_test_result_t;

typedef void (*mqtt_data_handle_t)(char* data, int len); 

void mqtt_app_start(void);
void json_gen_test(json_gen_test_result_t *result, char *key1, bool value1, char *key2, int value2, char *key3, char *value3);
void app_mqtt_set_data_callback(void * cb);
void app_mqtt_publish( char* topic,  char* data, int len);
void app_mqtt_subscriber(char *topic);

#endif /* __APP_MQTT_H__ */