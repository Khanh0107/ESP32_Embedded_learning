#ifndef __APP_MQTT_H__
#define __APP_MQTT_H__

typedef struct {
    char buf[256];
    int offset;
} json_gen_test_result_t;

void mqtt_app_start(void);
void json_gen_test(json_gen_test_result_t *result, char *key1, bool value1, char *key2, int value2, char *key3, char *value3);

#endif /* __APP_MQTT_H__ */