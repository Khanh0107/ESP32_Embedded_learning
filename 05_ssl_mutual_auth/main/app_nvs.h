#ifndef APP_NVS_H
#define APP_NVS_H

void app_nvs_set_value(char *key, int value);
void app_nvs_get_value(char *key, int *value);
void app_nvs_set_string(char *key, char* str);
void app_nvs_get_str(char *key, char *out);

#endif /* APP_NVS_H */