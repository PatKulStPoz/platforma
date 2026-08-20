#include <stdio.h>
#include <string.h>

#include "driver/gpio.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "control/driver.h"
#include "control/command.h"
#include "web/main.h"
#include "state.h"
#include <inttypes.h>
#include "pinout_config.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "datastorage.h"
// APP MAIN
extern "C" void app_main(void) {
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    gpio_install_isr_service(0);

    gpio_output_enable(STATUS_LED);

    DriverState* state = new DriverState();

    Driver* left = state->leftDriver();
    Driver* right = state->rightDriver();

    right->getConfig().driver_ticks_per_full_rotation =  243;
    right->getConfig().hall_sensor_ticks_per_full_rotation = 9;
    right->getConfig().has_brake = false;

    left->getConfig().driver_ticks_per_full_rotation = 75;
    left->getConfig().hall_sensor_ticks_per_full_rotation = 9;
    left->getConfig().has_brake = false;

    load_data("left_driver", &left->getConfig());
    load_data("right_driver", &right->getConfig());

    uartcmd_setup(state);

    web_setup(state);

    int tickOld = 0;
    int lastHal = 0;
    int tickOld2 = 0;
    int lastHal2 = 0;
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    uint32_t tick = 0;

    int ledTime = 100;

    while (true) {
        vTaskDelay(10 / portTICK_PERIOD_MS);

        ledTime = state->tickTasks() ? 20 : 100;

        gpio_set_level(STATUS_LED, (tick / ledTime) % 2 != 0);

        // Aktualizują stan sterownika
        left->update();
        right->update();

        //if (tick % 10 == 0) {
        //    printf("[DEBUG] Hal: %d, Driver: %d, Time: %" PRIu32 " ms, Last: %d, Dir: %d, Intr %d\n", right->getHalTicks(), right->getDriverTicksPerHal(), tick - tickOld, 
        //    right->getLastHall(), right->getHallDirection(), right->getIntrCount());
        //}

        int newHal = left->getHallTicks();
        if (newHal != lastHal) {
            printf("[STATE|Left] Hal: %d, Driver: %d, Time: %" PRIu32 "0 ms, Last: %d, Dir: %d\n", newHal, left->getDriverTicksPerHal(), tick - tickOld, left->getLastHall(), left->getHallDirection());
            lastHal = newHal;
            tickOld = tick;
        }
        newHal = right->getHallTicks();
        if (newHal != lastHal2) {
            printf("[STATE|Right] Hal: %d, Driver: %d, Time: %" PRIu32 "0 ms, Last: %d, Dir: %d\n", newHal, right->getDriverTicksPerHal(), tick - tickOld2, right->getLastHall(), right->getHallDirection());
            lastHal2 = newHal;
            tickOld2 = tick;
        }
        tick++;
    }
}

#include "control/command.cpp"
#include "control/workaround.cpp"
#include "util/stringreader.cpp"
#include "web/main.cpp"