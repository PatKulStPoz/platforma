#pragma once

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_http_server.h"
#include "sdkconfig.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_log.h"
#include <string>
#include "hotspot.h"
#include "page.h"
#include "../state.h"
#include "../control/command.h"

static httpd_handle_t server = nullptr;

static DriverState* static_driverState = NULL;

//websocket klienta
static int ws_fd = -1;

//lewe kolo
static int l_driverTicksFullRotation = 0;
static int l_halTicksFullRotation = 0;
static int l_driverTicks = 0;
static int l_rotationTick = 0;
static int l_driverTicksPerHal = 0;
static int l_halTicks = 0;

//prawe kolo
static int r_driverTicksFullRotation = 0;
static int r_halTicksFullRotation = 0;
static int r_driverTicks = 0;
static int r_rotationTick = 0;
static int r_driverTicksPerHal = 0;
static int r_halTicks = 0;

static void websocket_send_data(char* response) {
    if (ws_fd < 0)
    {
        return;
    }
    
    httpd_ws_frame_t ws_pkt = {};

    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    ws_pkt.payload = reinterpret_cast<uint8_t *>(response);
    ws_pkt.len = strlen(response);

    esp_err_t ret =
        httpd_ws_send_frame_async(
            server,
            ws_fd,
            &ws_pkt
        );

    if (ret != ESP_OK)
    {
        ESP_LOGW(
            TAG,
            "WebSocket send failed: %s",
            esp_err_to_name(ret)
        );

        ws_fd = -1;
    }
}

void websocket_print(std::string text) {
    char response[1024];

    snprintf(
            response,
            sizeof(response),
            R"rawliteral({
                "__type": "print",
                "text": "%s"
            })rawliteral",
            text.c_str()
        );

    //printf(("WS Response>" + text + "\n").c_str());

    websocket_send_data(response);
}

//websocket handler
static esp_err_t websocket_handler(httpd_req_t *req)
{
    //ESP_LOGI(TAG, "=== websocket_handler ===");
    //ESP_LOGI(TAG, "method = %d", req->method);
    //ESP_LOGI(TAG, "HTTP_GET = %d", HTTP_GET);
    ws_fd = httpd_req_to_sockfd(req);
    
    ESP_LOGI(TAG, "fd = %d", ws_fd);


    //ESP_LOGI(TAG, "=== WEBSOCKET FRAME ===");

    httpd_ws_frame_t ws_pkt = {};
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);

    ESP_LOGI(TAG,
             "recv_frame: ret=%s, len=%d, type=%d",
             esp_err_to_name(ret),
             ws_pkt.len,
             ws_pkt.type);

    if (ret != ESP_OK)
        return ret;

    if (ws_pkt.len > 0)
    {
        uint8_t *buf = new uint8_t[ws_pkt.len + 1];

        ws_pkt.payload = buf;

        ret = httpd_ws_recv_frame(
            req,
            &ws_pkt,
            ws_pkt.len
        );

        if (ret == ESP_OK)
        {
            buf[ws_pkt.len] = '\0';

            ESP_LOGI(TAG,
                     "Received: %s",
                     reinterpret_cast<char *>(buf));
        }

        std::string str = reinterpret_cast<char*>(buf);

        if (str.starts_with("EXEC>")) {
            parseAndExecuteMulti(static_driverState, websocket_print, str.substr(5));
        }

        delete[] buf;
    }

    return ret;
}

//wysylanie danych za pomoza websocket
static void websocket_send_update_data() {
    if (ws_fd < 0) {
        return;
    }

    char response[1024];

    snprintf(
            response,
            sizeof(response),
            R"rawliteral({
                "__type": "update_state",
                "l_driverTicksFullRotation": %d,
                "l_halTicksFullRotation": %d,
                "l_driverTicks": %d,
                "l_rotationTick": %d,
                "l_driverTicksPerHal": %d,
                "l_halTicks": %d,
                "r_driverTicksFullRotation": %d,
                "r_halTicksFullRotation": %d,
                "r_driverTicks": %d,
                "r_rotationTick": %d,
                "r_driverTicksPerHal": %d,
                "r_halTicks": %d
            })rawliteral",
            l_driverTicksFullRotation,
            l_halTicksFullRotation,
            l_driverTicks,
            l_rotationTick,
            l_driverTicksPerHal,
            l_halTicks,
            r_driverTicksFullRotation,
            r_halTicksFullRotation,
            r_driverTicks,
            r_rotationTick,
            r_driverTicksPerHal,
            r_halTicks
        );

    websocket_send_data(response);
}

// Start HTTP server
static httpd_handle_t start_web_server(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = PAGE_COUNT + 8  ;
    httpd_handle_t server = nullptr;
    if (httpd_start(&server, &config) == ESP_OK)
    {
        ESP_LOGI(
            TAG,
            "HTTP server started on port %d",
            config.server_port
        );
        
        init_pages(server);

        httpd_uri_t uri_root = {};

        uri_root.uri = "/";
        uri_root.method = HTTP_GET;
        uri_root.handler = page_get_file_page_html_handler;
        uri_root.user_ctx = nullptr;

        ESP_ERROR_CHECK(
            httpd_register_uri_handler(
                server,
                &uri_root
            )
        );

        httpd_uri_t ws_uri = {};

        ws_uri.uri = "/ws";
        ws_uri.method = HTTP_GET;
        ws_uri.handler = websocket_handler;
        ws_uri.user_ctx = nullptr;
        ws_uri.is_websocket = true;

        esp_err_t ws_ret = httpd_register_uri_handler(
        server,
        &ws_uri
        );

        ESP_LOGI(
            TAG,
            "Register /ws: %s",
            esp_err_to_name(ws_ret)
        );

ESP_ERROR_CHECK(ws_ret);

        return server;
    }

    ESP_LOGE(TAG, "Failed to start HTTP server");

    return nullptr;
}