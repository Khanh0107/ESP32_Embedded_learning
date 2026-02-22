idf_component_register(SRCS "hello_world_main.c"
                        INCLUDE_DIRS "."
                        PRIV_REQUIRES input_iot
                        PRIV_REQUIRES output_iot
                        PRIV_REQUIRES esp_driver_gpio
                       PRIV_REQUIRES spi_flash
                       INCLUDE_DIRS "")


