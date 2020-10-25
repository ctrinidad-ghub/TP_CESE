idf_component_register(SRCS "app_fsm.c" "app_adc.c" "main.c"
						SRCS "sapi_lcd.c"
						SRCS "sapi_convert.c"
						SRCS "app_adc.c"
						SRCS "app_lcd.c"
						SRCS "app_fsm"
                      INCLUDE_DIRS "../inc"
                      REQUIRES "esp_adc_cal"
                      REQUIRES "spi_flash"
                      REQUIRES "nvs_flash")
                   