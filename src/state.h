#pragma once

#include "control/driver.h"
#include "control/tasks.h"
#include <queue>


class DriverState {
    Driver* left;
    Driver* right;
    std::queue<BaseTask*> tasks;
    SemaphoreHandle_t taskSemaphore = NULL;

    public:
    DriverState() {
        this->left = new Driver(LEFT_DRIVER_PINS, 75, 9);
        this->right = new Driver(RIGHT_DRIVER_PINS, 243, 9);
        this->left->setup();
        this->right->setup();
        vSemaphoreCreateBinary( this->taskSemaphore );
    }

    ~DriverState() {
        this->left->destroy();
        this->right->destroy();

        delete this->left;
        delete this->right;
    }

    Driver* leftDriver() {
        return this->left;
    }

    Driver* rightDriver() {
        return this->right;
    }

    void pushTask(BaseTask* task) {
        xSemaphoreTake(this->taskSemaphore, portMAX_DELAY);
        this->tasks.push(task);
        xSemaphoreGive(this->taskSemaphore);
    }

    BaseTask* popTask() {
        xSemaphoreTake(this->taskSemaphore, portMAX_DELAY);
        BaseTask* task = this->tasks.front();
        this->tasks.pop();
        xSemaphoreGive(this->taskSemaphore);
        return task;
    }

    bool hasTasks() {
        xSemaphoreTake(this->taskSemaphore, portMAX_DELAY);
        bool state = !this->tasks.empty();
        xSemaphoreGive(this->taskSemaphore);
        return state;
    }
};
