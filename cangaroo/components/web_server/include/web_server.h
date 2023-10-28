#pragma once

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
