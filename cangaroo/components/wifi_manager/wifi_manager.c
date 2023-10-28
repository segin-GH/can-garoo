#include <stdio.h>
#include <wifi_manager.h>

static EventGroupHandle_t wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
#define MAXIMUM_RETRY 3

static void event_handler(void *args, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    static int connection_retry_count = 0;

    switch (event_id)
    {
    /* when wifi start's connecting as station */
    case WIFI_EVENT_STA_START:
        ESP_LOGI(TAG, "Connecting.....");
        esp_wifi_connect();
        break;

    /* when wifi is connected as station */
    case WIFI_EVENT_STA_CONNECTED:
        ESP_LOGI(TAG, "Connected");
        connection_retry_count = 0;
        break;

    /* when wifi is disconnected */
    case WIFI_EVENT_STA_DISCONNECTED:
        if (connection_retry_count < MAXIMUM_RETRY)
        {
            esp_wifi_connect();
            connection_retry_count++;
            ESP_LOGI(TAG, "Retrying to connect to %s", ((wifi_event_sta_disconnected_t *)event_data)->ssid);
        }
        else
        {
            xEventGroupSetBits(wifi_event_group, WIFI_FAIL_BIT);
            ESP_LOGE(TAG, "Failed to connect to %s", ((wifi_event_sta_disconnected_t *)event_data)->ssid);
        }
        break;

    /* when an IP addr is available  */
    case IP_EVENT_STA_GOT_IP:
        connection_retry_count = 0;
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
        break;

    case WIFI_EVENT_AP_START:
        ESP_LOGI(TAG, "AP Enabled");
        break;

    case WIFI_EVENT_AP_STOP:
        ESP_LOGI(TAG, "AP Disabled");
        break;

    /* nothing to be default */
    default:
        break;
    }
}

esp_err_t wifi_manager_init(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t wifi_init_cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_cfg));

    esp_event_handler_instance_t any_id_instance;
    esp_event_handler_instance_t got_ip_instance;

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &any_id_instance);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &got_ip_instance);

    return ESP_OK;
}

esp_err_t wifi_manager_connect_sta(const wifi_credentials_t *credentials, const int k_timeout)
{
    if (!credentials || strlen(credentials->ssid) == 0 || strlen(credentials->password) == 0)
    {
        ESP_LOGE(TAG, "Invalid credentials");
        return ESP_ERR_INVALID_ARG;
    }

    esp_netif_create_default_wifi_sta();

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "",
            .password = "",
        },
    };

    strncpy((char *)wifi_config.sta.ssid, credentials->ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char *)wifi_config.sta.password, credentials->password, sizeof(wifi_config.sta.password));

    esp_err_t err;

    err = esp_wifi_set_mode(WIFI_MODE_STA);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set WIFI mode to STA");
        return err;
    }

    err = esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set WIFI configuration");
        return err;
    }

    err = esp_wifi_start();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start WIFI");
        return err;
    }

    ESP_LOGI(TAG, "Connecting to %s", credentials->ssid);
    EventBits_t bits = xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdTRUE, pdFALSE, k_timeout / portTICK_PERIOD_MS);
    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(TAG, "Connected to %s", credentials->ssid);
        return ESP_OK;
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        return ESP_FAIL;
    }
    else
    {
        ESP_LOGE(TAG, "Connection timed out");
        return ESP_ERR_TIMEOUT;
    }
}

void wifi_manager_disconnect(void)
{
}

esp_err_t wifi_manager_deinit(void)
{
    return ESP_OK;
}
