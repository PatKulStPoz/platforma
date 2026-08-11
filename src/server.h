#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_http_server.h"
#include "sdkconfig.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_log.h"

#include "page.h"
#include "hotspot.h"

static httpd_handle_t server = nullptr;

//websocket klienta
static int ws_fd = -1;

//lewe kolo
static int l_PWM_counter = 0;
static int l_hall_counter = 0;
static bool l_state = true;

//prawe kolo
static int r_PWM_counter = 0;
static int r_hall_counter = 0;
static bool r_state = true;

// HTTP GET: /
static esp_err_t root_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html; charset=UTF-8");

    httpd_resp_send(
        req,
        html_page,
        HTTPD_RESP_USE_STRLEN
    );

    return ESP_OK;
}


static esp_err_t websocket_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "=== websocket_handler ===");
    ESP_LOGI(TAG, "method = %d", req->method);
    ESP_LOGI(TAG, "HTTP_GET = %d", HTTP_GET);
    ws_fd = httpd_req_to_sockfd(req);
    
    ESP_LOGI(TAG, "fd = %d", ws_fd);


    ESP_LOGI(TAG, "=== WEBSOCKET FRAME ===");

    httpd_ws_frame_t ws_pkt = {};
    
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

        delete[] buf;
    }

    return ret;
}

//wysylanie danych za pomoza websocket
static void websocket_send_data()
{
    if (ws_fd < 0)
    {
        return;
    }

    char response[256];

    snprintf(
            response,
            sizeof(response),
            R"rawliteral({
                "l_PWM_counter": %d,
                "l_hall_counter": %d,
                "l_state": %s,
                "r_PWM_counter": %d,
                "r_hall_counter": %d,
                "r_state": %s
            })rawliteral",
            l_PWM_counter,
            l_hall_counter,
            l_state ? "true" : "false",
            r_PWM_counter,
            r_hall_counter,
            r_state ? "true" : "false"
        );

    httpd_ws_frame_t ws_pkt = {};

    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    ws_pkt.payload =
        reinterpret_cast<uint8_t *>(response);
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

// Start HTTP server
static httpd_handle_t start_web_server(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = nullptr;
    if (httpd_start(&server, &config) == ESP_OK)
    {
        ESP_LOGI(
            TAG,
            "HTTP server started on port %d",
            config.server_port
        );
        httpd_uri_t uri_root = {};

        uri_root.uri = "/";
        uri_root.method = HTTP_GET;
        uri_root.handler = root_get_handler;
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