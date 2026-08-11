#include <stdio.h>
#include <string.h>

#include "hotspot.h"
#include "server.h"

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
    
    xTaskCreate(
    websocket_task,
    "websocket_task",
    4096,
    nullptr,
    5,
    nullptr
    );
}