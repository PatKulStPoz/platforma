#include <stdio.h>
#include <string.h>

#include "hotspot.h"
#include "server.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "../control/driver.h"
#include "../control/command.h"
#include <queue>
//#include "mdns.h"

struct DriverPair {
    Driver* left;
    Driver* right;
};

void update_info(Driver* left, Driver* right)
{
    l_driverTicksFullRotation = left->getDriverTicksFullRotation();
    l_halTicksFullRotation = left->getHalTicksFullRotation();
    l_driverTicks = left->getDriverTicks();
    l_rotationTick = left->getRotationTick();
    l_driverTicksPerHal = left->getDriverTicksPerHal();
    l_halTicks = left->getHalTicks();

    r_driverTicksFullRotation = right->getDriverTicksFullRotation();
    r_halTicksFullRotation = right->getHalTicksFullRotation();
    r_driverTicks = right->getDriverTicks();
    r_rotationTick = right->getRotationTick();
    r_driverTicksPerHal = right->getDriverTicksPerHal();
    r_halTicks = right->getHalTicks();
}

static void websocket_task(void *arg)
{
    ESP_LOGI(
            TAG,
            "Task started!"
        );
    while (true)
    {
        update_info(
            static_cast<DriverPair*>(arg)->left,
            static_cast<DriverPair*>(arg)->right
        );
        websocket_send_data();
        vTaskDelay(
            pdMS_TO_TICKS(1000)
        );
    }
}

void web_setup(Driver* left, Driver* right) {
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
            "========================================"
        );
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
        ESP_LOGI(
            TAG,
            "========================================"
        );
    }

    /*ESP_ERROR_CHECK(mdns_init());
    //set mDNS hostname (required if you want to advertise services)
    ESP_ERROR_CHECK(mdns_hostname_set(ESP_MDNS_NAME));
    ESP_LOGI(TAG, "mdns hostname set to: [%s]", hostname);
    //set default mDNS instance name
    ESP_ERROR_CHECK(mdns_instance_name_set(ESP_MDNS_NAME));

    mdns_service_add("ESP32-WebServer", "_http", "_tcp", 80, NULL, 3);*/

    auto* drivers = new DriverPair{left, right};

    xTaskCreate(websocket_task, "websocket_task", 4096, drivers, 5, nullptr);
}