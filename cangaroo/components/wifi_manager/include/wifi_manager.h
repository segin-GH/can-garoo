/* TODO add  disconnect and deinit function */

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_err.h>
#include <esp_netif.h>
#include <esp_wifi.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <nvs_flash.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define TAG "WIFI_MANAGER"
#define MAX_SSID_LENGTH 32
#define MAX_PASSWORD_LENGTH 64

    /* Structure to hold WiFi credentials */
    typedef struct
    {
        char ssid[MAX_SSID_LENGTH];
        char password[MAX_PASSWORD_LENGTH];
    } wifi_credentials_t;

    esp_err_t wifi_manager_init(void);

    esp_err_t wifi_manager_connect_sta(const wifi_credentials_t *credentials, const int k_timeout);

    /* Only use this to disconnect it turn of wifi no resource will be cleaned */
    void wifi_manager_disconnect(void);

    /* disconnects and complete clean up of all resources */
    esp_err_t wifi_manager_deinit(void);

#ifdef __cplusplus
}
#endif