idf_component_register(SRCS "app_adc.c" "main.c"
						SRCS "sapi_lcd.c"
						SRCS "sapi_convert.c"
						SRCS "app_adc.c"
						SRCS "app_lcd.c"
                      INCLUDE_DIRS "../inc"
                      REQUIRES "esp_adc_cal"
                      REQUIRES "spi_flash"
                      REQUIRES "nvs_flash")
                   