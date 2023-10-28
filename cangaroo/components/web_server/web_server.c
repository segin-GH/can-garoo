/* TODO Make webpage loading fast*/
/* TODO mv the init of spiffs to init server and copy the file into buffer
        before the default uri request comes */

#include <stdio.h>
#include "web_server.h"
#include <esp_spiffs.h>

static const char *HTTP_TAG = "HTTP_SERVER";
static httpd_handle_t server = NULL;

static esp_err_t default_url(httpd_req_t *req)
{
    esp_err_t err;

    esp_vfs_spiffs_conf_t esp_vfs_spiffs_config = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true};
    err = esp_vfs_spiffs_register(&esp_vfs_spiffs_config);
    if (err != ESP_OK)
    {
        ESP_LOGE(HTTP_TAG, "Failed to mount or format filesystem");
        return ESP_FAIL;
    }

    char path[600];
    if (strcmp(req->uri, "/") == 0)
    {
        strcpy(path, "/spiffs/index.html");
    }
    else
    {
        sprintf(path, "/spiffs%s", req->uri);
    }
    char *ext = strrchr(path, '.');
    if (strcmp(ext, ".css") == 0)
    {
        httpd_resp_set_type(req, "text/css");
    }
    if (strcmp(ext, ".js") == 0)
    {
        httpd_resp_set_type(req, "text/javascript");
    }

    FILE *file = fopen(path, "r");
    if (file == NULL)
    {
        httpd_resp_send_404(req);
        esp_vfs_spiffs_unregister(NULL);
        return ESP_OK;
    }
    char lineRead[256];
    while (fgets(lineRead, sizeof(lineRead), file))
    {
        httpd_resp_sendstr_chunk(req, lineRead);
    }

    httpd_resp_sendstr_chunk(req, NULL);
    esp_vfs_spiffs_unregister(NULL);
    return ESP_OK;
}

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
    esp_err_t err;
    err = esp_spiffs_check("spiffs");
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;

    httpd_uri_t root = {
        .uri = "/*",
        .method = HTTP_GET,
        .handler = default_url,
        .user_ctx = NULL};


    // Start the HTTP Server
    err = httpd_start(&server, &config);
    if (err != ESP_OK)
    {
        ESP_LOGE(HTTP_TAG, "Error starting server: %s", esp_err_to_name(err));
        return err;
    }

    // Register URI handler
    err = httpd_register_uri_handler(server, &root);
    if (err != ESP_OK)
    {
        ESP_LOGE(HTTP_TAG, "Error registering URI handler: %s", esp_err_to_name(err));
        deinit_http_server();
        return err;
    }

    ESP_LOGI(HTTP_TAG, "Web-server started successfully.");
    return ESP_OK;
}