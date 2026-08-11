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
#include "command.h"
#include <queue>

extern "C" {

void app_main(void) {
    printf("Start!\n");
    gpio_install_isr_service(ESP_INTR_FLAG_EDGE);
    driver_prepare();

    Driver* left = new Driver(LEFT_DRIVER_PINS, 75, 9);
    Driver* right = new Driver(RIGHT_DRIVER_PINS, 243, 1);
    left->setup();
    right->setup();
    printf("Waiting!\n");
    left->setLevel(0);
    right->setLevel(0);

    int tickOld = 0;
    int lastHal = 0;
    vTaskDelay(3000 / portTICK_PERIOD_MS);

    std::queue<CommandTask*> tasks;

    tasks.push(new SetLevelTask(255));
    tasks.push(new RepeatTasks({
        /*new WaitTicksTask(3 * 100),
        new RotateTask(360 / 9),
        new WaitForFinishedTask(),
        new WaitTicksTask(3 * 100),
        new RotateTask(360),
        new WaitForFinishedTask(),
        new WaitTicksTask(3 * 100),
        new RotateTask(360 * 2),
        new WaitForFinishedTask(),
        new WaitTicksTask(3 * 100),
        new RotateTask(360 / 3),
        new WaitForFinishedTask(),*/
        new RotateTask(360 * 2),
        new WaitForFinishedTask(),
        new WaitTicksTask(3 * 100),
    }, 1000));

    tasks.push(new WaitTicksTask(4 * 100));
    tasks.push(new RotateTask(180, 100));
    tasks.push(new WaitForFinishedTask());

    CommandTask* currentTask = NULL;
    int tick = 0;
    while (true) {
        vTaskDelay(10 / portTICK_PERIOD_MS);

        while(true) {

            if (currentTask == NULL && !tasks.empty()) {
                tick = 0;
                currentTask = tasks.front();
                tasks.pop();
                currentTask->setup(left, right);
                printf("Task Changed!\n");
            } else if (currentTask == NULL && tasks.empty()) {
                break;
            }

            if (currentTask != NULL && currentTask->update(left, right, tick)) {
                delete currentTask;
                currentTask = NULL;
                printf("Task Finished!\n");
            } else {
                break;
            }
        }

        left->update();
        right->update();

        int newHal = left->getHalTicks();
        if (newHal != lastHal) {
            printf("Hal: %d, Driver: %d, Time: %d0 ms\n", newHal, left->getDriverTicksPerHal(), tick - tickOld);
            lastHal = newHal;
            tickOld = tick;
        }
        tick++;
    }
}

}