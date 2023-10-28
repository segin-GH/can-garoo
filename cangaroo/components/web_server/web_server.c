
#include <stdio.h>
#include "web_server.h"

static const char *HTTP_TAG = "HTTP_SERVER";
static httpd_handle_t server = NULL;

static esp_err_t default_url(httpd_req_t *req)
{
    char resp_str[100];
    snprintf(resp_str, sizeof(resp_str), "Hello from ESP32!");

    // Send response
    esp_err_t ret = httpd_resp_send(req, resp_str, strlen(resp_str));
    if (ret != ESP_OK)
    {
        ESP_LOGE(HTTP_TAG, "Failed to send response: %s", esp_err_to_name(ret));
    }

    return ret;
}

httpd_uri_t root = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = default_url,
    .user_ctx = NULL};

void deinit_http_server(void)
{
    if (server)
    {
        ESP_LOGI(HTTP_TAG, "Stopping web server");
        httpd_stop(server);
        server = NULL;
    }
}

esp_err_t init_http_server(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the HTTP Server
    esp_err_t ret = httpd_start(&server, &config);
    if (ret != ESP_OK)
    {
        ESP_LOGE(HTTP_TAG, "Error starting server: %s", esp_err_to_name(ret));
        return ret;
    }

    // Register URI handler
    ret = httpd_register_uri_handler(server, &root);
    if (ret != ESP_OK)
    {
        ESP_LOGE(HTTP_TAG, "Error registering URI handler: %s", esp_err_to_name(ret));
        deinit_http_server();
        return ret;
    }

    ESP_LOGI(HTTP_TAG, "Web-server started successfully.");
    return ESP_OK;
}