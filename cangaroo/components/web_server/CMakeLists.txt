idf_component_register(SRCS "web_server.c"
                    REQUIRES esp_http_server spiffs
                    INCLUDE_DIRS "include")
