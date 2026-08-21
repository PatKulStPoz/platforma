#include <stdio.h>
#include <string.h>

#include "main.h"
#include "wifi.h"
#include "server.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "../control/driver.h"
#include <queue>
#include "mdns.h"
#include "../state.h"
#include "../control/command.h"
#include "simproto.h"

static void websocket_task(void *arg) {
    ESP_LOGI(
            TAG,
            "Task started!"
        );
    while (true)
    {
        websocket_send_update_data(static_cast<DriverState*>(arg));
        vTaskDelay(
            pdMS_TO_TICKS(500)
        );
    }
}

void web_setup(DriverState* state) {
    static_driverState = state;
    //Initialize NVS

    ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
    if (AP_WIFI_ENABLED) {
        wifi_init_softap();
    } else if (CONN_WIFI_ENABLED) {
        wifi_init_sta();
    }

    // Start web server
    server = start_web_server();

    if (server != nullptr)
    {
        ESP_LOGI(
            TAG,
            "========================================"
        );
        ESP_LOGI(
            TAG,
            "Web Server started!"
        );

        if (AP_WIFI_ENABLED) {
            ESP_LOGI(
                TAG,
                "Connect to WiFi: %s",
                AP_WIFI_SSID
            );
            ESP_LOGI(
                TAG,
                "Password: %s",
                AP_WIFI_PASS
            );
        }
        ESP_LOGI(
            TAG,
            "Open: http://%s.local",
            MDNS_HOSTNAME
        );
        ESP_LOGI(
            TAG,
            "========================================"
        );
    }

    setup_textovertpc(state);

    ESP_ERROR_CHECK(mdns_init());
    //set mDNS hostname (required if you want to advertise services)
    ESP_ERROR_CHECK(mdns_hostname_set(MDNS_HOSTNAME));
    ESP_LOGI(TAG, "mdns hostname set to: [%s]", MDNS_HOSTNAME);
    //set default mDNS instance name
    ESP_ERROR_CHECK(mdns_instance_name_set(MDNS_NAME));

    mdns_service_add("ESP32-WebServer", "_http", "_tcp", 80, NULL, 0);
    mdns_service_add("ESP32-WebServer", "_text", "_tcp", 2000, NULL, 0);

    xTaskCreate(websocket_task, "websocket_task", 4096, state, 5, nullptr);
}