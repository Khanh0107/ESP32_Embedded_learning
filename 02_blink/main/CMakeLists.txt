idf_component_register(
    SRCS "app_main.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES input_iot
    PRIV_REQUIRES output_iot
    PRIV_REQUIRES esp_driver_gpio
)
