#include <stdio.h>
#include <string.h>

#include "hotspot.h"
#include "server.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver.h"
#include "command.h"
#include <queue>

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
    printf("Start!\n");
    gpio_install_isr_service(ESP_INTR_FLAG_EDGE);
    driver_prepare();

    Driver* left = new Driver(LEFT_DRIVER_PINS, 75, 9);
    Driver* right = new Driver(RIGHT_DRIVER_PINS, 243, 9);

    auto* drivers = new DriverPair{left, right};

    xTaskCreate(
    websocket_task,
    "websocket_task",
    4096,
    drivers,
    5,
    nullptr
    );

    left->setup();
    right->setup();
    printf("Waiting!\n");
    left->setLevel(0);
    right->setLevel(0);

    int tickOld = 0;
    int lastHal = 0;
    int tickOld2 = 0;
    int lastHal2 = 0;
    vTaskDelay(3000 / portTICK_PERIOD_MS);

    std::queue<CommandTask*> tasks;

    tasks.push(new SetLevelTask(255));
    tasks.push(new RepeatTasks({
        /*new RotateTask(360 * 2),
        new WaitForFinishedTask(),
        new WaitTicksTask(3 * 100),
        new RotateTask(CMD_DRIVER_LEFT, 360),
        new WaitForFinishedTask(),
        new WaitTicksTask(3 * 100),
        new RotateTask(CMD_DRIVER_RIGHT, 360),
        new WaitForFinishedTask(),
        new WaitTicksTask(10 * 100),
        new SetDirectionTask(DRIVER_BACKWARDS),
        new RotateTask(120),
        new WaitForFinishedTask(),
        new SetDirectionTask(DRIVER_FORWARD),
        new WaitTicksTask(3 * 100),*/

        /*new StartTask(),
        new WaitTicksTask(6 * 100),
        new StopTask(),
        new WaitTicksTask(4 * 100),
        new SetDirectionTask(DRIVER_BACKWARDS),
        new StartTask(),
        new WaitTicksTask(6 * 100),
        new StopTask(),
        new WaitTicksTask(4 * 100),
        new SetDirectionTask(DRIVER_FORWARD),*/
    }, 1000));

    tasks.push(new WaitTicksTask(4 * 100));
    tasks.push(new RotateTask(180));
    tasks.push(new WaitForFinishedTask());

    CommandTask* currentTask = NULL;
    int tick = 0;
    int taskTick = 0;
    while (true) {
        vTaskDelay(10 / portTICK_PERIOD_MS);

        while(true) {

            if (currentTask == NULL && !tasks.empty()) {
                taskTick = 0;
                currentTask = tasks.front();
                tasks.pop();
                currentTask->setup(left, right);
                printf("Task Changed!\n");
            } else if (currentTask == NULL && tasks.empty()) {
                break;
            }

            if (currentTask != NULL && currentTask->update(left, right, taskTick)) {
                delete currentTask;
                currentTask = NULL;
                printf("Task Finished!\n");
            } else {
                break;
            }
        }

        // Aktualizują stan sterownika
        left->update();
        right->update();


        int newHal = left->getHalTicks();
        if (newHal != lastHal) {
            printf("Left> Hal: %d, Driver: %d, Time: %d0 ms\n", newHal, left->getDriverTicksPerHal(), taskTick - tickOld);
            lastHal = newHal;
            tickOld = tick;
        }
        newHal = right->getHalTicks();
        if (newHal != lastHal2) {
            printf("Right> Hal: %d, Driver: %d, Time: %d0 ms\n", newHal, right->getDriverTicksPerHal(), taskTick - tickOld2);
            lastHal2 = newHal;
            tickOld2 = tick;
        }
        tick++;
        taskTick++;
    }
}