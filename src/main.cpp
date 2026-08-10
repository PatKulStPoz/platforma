/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver.h"

extern "C" {

void app_main(void) {
    printf("Start!\n");
    gpio_install_isr_service(ESP_INTR_FLAG_EDGE);
    driver_prepare();

    Driver* left = new Driver(LEFT_DRIVER_PINS, 85, 1);
    left->setup();
    printf("Waiting!\n");
    left->setLevel(0);

    int tickOld = 0;
    int tick = 0;
    int lastHal = 0;
    
    while (true) {
        vTaskDelay(10 / portTICK_PERIOD_MS);

        int newHal = left->getHalTicks();
        if (newHal != lastHal) {
            printf("Hal: %d, Driver: %d, Time: %d0 ms\n", newHal, left->getDriverTicksPerHal(), tick - tickOld);
            lastHal = newHal;
            tickOld = tick;
        }
        tick++;

        /*if (!left->isAutomatic()) {
            printf("Czekaj...\n");
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            left->rotateDeg(360 * 2);
            left->setLevel(255);
            printf("Startuje...\n");
            tick = 0;
        }*/

        // if (tick == 1500) {
        //     left->setLevel(0);
        //     printf("Stop!\n");
        // } else if (tick == 2000) {
        //     printf("Start!\n");
        //     left->setLevel(255);
        //     left->clearCount();
        //     tick = 0;
        // }
    }
}

}