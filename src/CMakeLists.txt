idf_component_register(SRCS "main.c"
						SRCS "sapi_lcd.c"
						SRCS "sapi_convert.c"
						SRCS "adc.c"
                      INCLUDE_DIRS "../inc"
                      REQUIRES "esp_adc_cal"
                      REQUIRES "spi_flash"
                      REQUIRES "nvs_flash")
                   