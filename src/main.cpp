#include <stdio.h>
#include <string.h>

#include "driver/gpio.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "control/driver.h"
#include "web/main.cpp"
#include "state.h"
#include <inttypes.h>

// APP MAIN
extern "C" void app_main(void) {
    gpio_install_isr_service(ESP_INTR_FLAG_EDGE);
    driver_prepare();

    DriverState* state = new DriverState();

    Driver* left = state->leftDriver();
    Driver* right = state->rightDriver();

    left->setLevel(0);
    right->setLevel(0);

    web_setup(state);

    int tickOld = 0;
    int lastHal = 0;
    int tickOld2 = 0;
    int lastHal2 = 0;
    vTaskDelay(3000 / portTICK_PERIOD_MS);

    //tasks.push(new SetDirectionTask(DRIVER_BACKWARDS));
    //tasks.push(new SetLevelTask(100));
    //tasks.push(new StartTask());


    BaseTask* currentTask = NULL;
    int tick = 0;
    uint32_t taskTick = 0;
    while (true) {
        vTaskDelay(10 / portTICK_PERIOD_MS);

        while(true) {
            if (currentTask == NULL && state->hasTasks()) {
                taskTick = 0;
                currentTask = state->popTask();
                currentTask->setup(left, right);
                printf("Task Changed!\n");
            } else if (currentTask == NULL && !state->hasTasks()) {
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

        /*if (tick % 10 == 0) {
            printf("Left> Hal: %d, Driver: %d, Time: %d0 ms, Last: %d, Dir: %d, Intr %d\n", left->getHalTicks(), left->getDriverTicksPerHal(), taskTick - tickOld, 
           left->getLastHall(), left->getHallDirection(), left->getIntrCount());
        }*/

        int newHal = left->getHalTicks();
        if (newHal != lastHal) {
            printf("Left> Hal: %d, Driver: %d, Time: %" PRIu32 "0 ms, Last: %d, Dir: %d\n", newHal, left->getDriverTicksPerHal(), taskTick - tickOld, left->getLastHall(), left->getHallDirection());
            lastHal = newHal;
            tickOld = tick;
        }
        newHal = right->getHalTicks();
        if (newHal != lastHal2) {
            printf("Right> Hal: %d, Driver: %d, Time: %" PRIu32 "0 ms, Last: %d, Dir: %d\n", newHal, right->getDriverTicksPerHal(), taskTick - tickOld2, right->getLastHall(), right->getHallDirection());
            lastHal2 = newHal;
            tickOld2 = tick;
        }
        tick++;
        taskTick++;
    }
}