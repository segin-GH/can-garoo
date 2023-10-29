/* TODO Make webpage loading fast*/
/* TODO mv the init of spiffs to init server and copy the file into buffer
        before the default uri request comes */
/* TODO when a unknown route is called the the core panics */

#include <stdio.h>
#include "web_server.h"
#include <esp_spiffs.h>

static const char *HTTP_TAG = "HTTP_SERVER";
static httpd_handle_t server = NULL;

#define WS_MAX_SIZE 1024
int client_session_id;

static esp_err_t send_text_response(httpd_req_t *req, const char *response_text);
static esp_err_t handle_web_socket_frame(httpd_req_t *req);

static esp_err_t on_web_socket_url(httpd_req_t *req)
{
    client_session_id = httpd_req_to_sockfd(req);
    if (req->method == HTTP_GET)
    {
        return ESP_OK;
    }

    esp_err_t err = handle_web_socket_frame(req);
    if (err != ESP_OK)
    {
        ESP_LOGE(HTTP_TAG, "Failed to handle websocket frame (%s)", esp_err_to_name(err));
    }

    return err;
}

static esp_err_t handle_web_socket_frame(httpd_req_t *req)
{
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));

    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    ws_pkt.payload = malloc(WS_MAX_SIZE);
    if (ws_pkt.payload == NULL)
    {
        return ESP_ERR_NO_MEM;
    }

    esp_err_t err = httpd_ws_recv_frame(req, &ws_pkt, WS_MAX_SIZE);
    if (err != ESP_OK)
    {
        free(ws_pkt.payload);
        return err;
    }

    printf("payload: %.*s\n", ws_pkt.len, ws_pkt.payload);
    free(ws_pkt.payload);

    return send_text_response(req, "connected ok :) ");
}

static esp_err_t send_text_response(httpd_req_t *req, const char *response_text)
{
    httpd_ws_frame_t ws_response = {
        .final = true,
        .fragmented = false,
        .type = HTTPD_WS_TYPE_TEXT,
        .payload = (uint8_t *)response_text,
        .len = strlen(response_text),
    };

    return httpd_ws_send_frame(req, &ws_response);
}
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

    err = httpd_start(&server, &config);
    if (err != ESP_OK)
    {
        ESP_LOGE(HTTP_TAG, "Error starting server: %s", esp_err_to_name(err));
        return err;
    }
    httpd_uri_t web_socket_url = {
        .uri = "/ws",
        .method = HTTP_GET,
        .handler = on_web_socket_url,
        .is_websocket = true};
    err = httpd_register_uri_handler(server, &web_socket_url);
    if (err != ESP_OK)
    {
        ESP_LOGE(HTTP_TAG, "Error registering URI handler: %s", esp_err_to_name(err));
        deinit_http_server();
        return err;
    }

    httpd_uri_t root = {
        .uri = "/*",
        .method = HTTP_GET,
        .handler = default_url,
        .user_ctx = NULL};
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