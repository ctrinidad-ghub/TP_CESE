idf_component_register(SRCS "main.c"
						SRCS "sapi_lcd.c"
						SRCS "sapi_convert.c"
						SRCS "app_adc.c"
						SRCS "app_lcd.c"
						SRCS "app_fsm.c"
						SRCS "app_uart.c"
						SRCS "teclas.c"
                      INCLUDE_DIRS "../inc"
                      REQUIRES "esp_adc_cal"
                      REQUIRES "spi_flash"
                      REQUIRES "nvs_flash")
                   