#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "sdkconfig.h"
#include "esp_mac.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#define ESP_WIFI_SSID      "mywifissid"
#define ESP_WIFI_PASS      "mywifipass"
#define EXAMPLE_ESP_WIFI_CHANNEL   1
#define EXAMPLE_MAX_STA_CONN       4

static const char *TAG = "wifi softAP";

static httpd_handle_t server = nullptr;

//lewe kolo
static int l_PWM_counter = 0;
static int l_hall_counter = 0;
static bool l_state = true;

//prawe kolo
static int r_PWM_counter = 0;
static int r_hall_counter = 0;
static bool r_state = true;

//websocket klienta
static int ws_fd = -1;

static const char *html_page = R"raw(
<!DOCTYPE html>
<html>
  <head>
    <title>Status platformy</title>
  </head>
  <body>
    <h3>
    WebSocket:
    <span id="ws-status">Łączenie...</span>
    </h3>

    <h3>Lewe koło</h3>
    <p>Status: <span id="l_state">OFF</span></p>
    <p>Licznik impulsów: <span id="l_PWM_counter">--</span></p>
    <p>Licznik impulsów (Hall): <span id="l_hall_counter">--</span></p>

    <h3>Prawe koło</h3>
    <p>Status: <span id="r_state">OFF</span></p>
    <p>Licznik impulsów: <span id="r_PWM_counter">--</span></p>
    <p>Licznik impulsów (Hall): <span id="r_hall_counter">--</span></p>

    <script>
        let socket = new WebSocket("ws://" + window.location.host + "/ws");
        socket.onopen = function() {
            console.log("WebSocket connected");
            document.getElementById("ws-status").textContent = "POŁĄCZONO";
            socket.send("CLIENT CONNECTED");
        };

        socket.onmessage = function(event) {
            console.log("ESP32:", event.data);
        };

        socket.onerror = function(error) {
            console.error("WebSocket error:", error);
            document.getElementById("ws-status").textContent = "BŁĄD";
        };

        socket.onclose = function() {
            console.log("WebSocket disconnected");
            document.getElementById("ws-status").textContent = "ROZŁĄCZONO";
        };

        socket.onmessage = function(event) {

        const data = JSON.parse(event.data);

        console.log("Licznik impulsów(L):", data.l_PWM_counter);
        console.log("Licznik impulsów (Hall)(L):", data.l_hall_counter);
        console.log("Status(L):", data.l_state);

        console.log("Licznik impulsów(R):", data.r_PWM_counter);
        console.log("Licznik impulsów (Hall)(R):", data.r_hall_counter);
        console.log("Status(R):", data.r_state);

        document.getElementById("l_PWM_counter").textContent =
            data.l_PWM_counter;

        document.getElementById("l_hall_counter").textContent =
            data.l_hall_counter;

        document.getElementById('l_state').textContent =
            data.l_state ? 'ON' : 'OFF';
        
        document.getElementById("r_PWM_counter").textContent =
            data.r_PWM_counter;

        document.getElementById("r_hall_counter").textContent =
            data.r_hall_counter;

        document.getElementById('r_state').textContent =
            data.r_state ? 'ON' : 'OFF';
        };
  
    </script>

  </body>
</html>
)raw";

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


// Wi-Fi / IP event handler
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station " MACSTR " join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}

void wifi_init_softap(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config{};

    strncpy(
        reinterpret_cast<char *>(wifi_config.ap.ssid),
        ESP_WIFI_SSID,
        sizeof(wifi_config.ap.ssid) - 1
    );

    strncpy(
        reinterpret_cast<char *>(wifi_config.ap.password),
        ESP_WIFI_PASS,
        sizeof(wifi_config.ap.password) - 1
    );

    wifi_config.ap.ssid_len =
        strlen(ESP_WIFI_SSID);

    wifi_config.ap.channel = 1;
    wifi_config.ap.max_connection = 4;
    wifi_config.ap.authmode = WIFI_AUTH_WPA2_PSK;


    if (strlen(ESP_WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             ESP_WIFI_SSID, ESP_WIFI_PASS, EXAMPLE_ESP_WIFI_CHANNEL);
}

static void websocket_task(void *arg)
{
    ESP_LOGI(
            TAG,
            "Task started!"
        );
    while (true)
    {
        l_PWM_counter += 1;
        r_PWM_counter +=1;
        websocket_send_data();

        vTaskDelay(
            pdMS_TO_TICKS(1000)
        );
    }
}

// APP MAIN
extern "C" void app_main(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
    wifi_init_softap();

    // Start web server
    server = start_web_server();

    if (server != nullptr)
    {
        ESP_LOGI(
            TAG,
            "Web Server started!"
        );

        ESP_LOGI(
            TAG,
            "Connect to WiFi: %s",
            ESP_WIFI_SSID
        );

        ESP_LOGI(
            TAG,
            "Password: %s",
            ESP_WIFI_PASS
        );

        ESP_LOGI(
            TAG,
            "Open: http://192.168.4.1/"
        );
    }
    
    xTaskCreate(
    websocket_task,
    "websocket_task",
    4096,
    nullptr,
    5,
    nullptr
    );
}