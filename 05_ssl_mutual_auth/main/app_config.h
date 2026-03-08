#ifndef APP_CONFIG_H
#define APP_CONFIG_H

typedef enum{
    PROVISION_ACCESSPOINT = 0,
    PROVISION_SMARTCONFIG = 1
} provision_type_t;

void app_config(void);

#endif // APP_CONFIG_H