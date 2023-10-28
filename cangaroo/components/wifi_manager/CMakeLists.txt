idf_component_register(SRCS "wifi_manager.c"
                    # PRIV_REQUIRES nvs_flash esp_wifi esp_event
                    REQUIRES nvs_flash esp_wifi esp_event
                    INCLUDE_DIRS "include")
