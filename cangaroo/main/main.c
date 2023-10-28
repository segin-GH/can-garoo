#include <stdio.h>
#include <wifi_manager.h>
#include "web_server.h"

void app_main(void)
{
    wifi_credentials_t wifi_credentials = {
        .ssid = "sussy_baka",
        .password = "luffy@gear5",
    };

    wifi_manager_init();
    wifi_manager_connect_sta(&wifi_credentials, 10000);

    init_http_server();
}


