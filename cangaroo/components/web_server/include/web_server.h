#pragma once

#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
// #include <nvs_flash.h>
#include <sys/param.h>
#include "esp_netif.h"
#include "esp_eth.h"
#include <stdio.h>
#include "esp_http_server.h"
#include "esp_log.h"

#ifdef __cplusplus
extern "C"
{
#endif

    esp_err_t init_http_server(void);
    void deinit_http_server(void);

#ifdef __cplusplus
}
#endif
