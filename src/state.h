#pragma once

#include "control/driver.h"
#include "control/tasks.h"
#include <queue>


class DriverState {
    Driver* left;
    Driver* right;
    std::queue<BaseTask*> tasks;
    BaseTask* currentTask = NULL;
    SemaphoreHandle_t taskSemaphore = NULL;
    uint32_t taskTick = 0;

    public:
    DriverState() {
        this->left = new Driver(LEFT_DRIVER_PINS);
        this->right = new Driver(RIGHT_DRIVER_PINS);
        this->left->setup();
        this->right->setup();
        vSemaphoreCreateBinary( this->taskSemaphore );
    }

    ~DriverState() {
        this->clearTasks();
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

    bool hasTasks() {
        xSemaphoreTake(this->taskSemaphore, portMAX_DELAY);
        bool state = !this->tasks.empty();
        xSemaphoreGive(this->taskSemaphore);
        return state;
    }

    void clearTasks() {
        xSemaphoreTake(this->taskSemaphore, portMAX_DELAY);
        if (this->currentTask != NULL) {
            delete this->currentTask;
            this->currentTask = NULL;
        }

        while (!this->tasks.empty()) {
            delete this->tasks.front();
            this->tasks.pop();
        }

        xSemaphoreGive(this->taskSemaphore);
    }

    bool tickTasks() {
        xSemaphoreTake(this->taskSemaphore, portMAX_DELAY);
        bool ret = false;

        while(true) {
            if (this->currentTask == NULL && !this->tasks.empty()) {
                this->currentTask = this->tasks.front();
                this->tasks.pop();

                this->currentTask->setup(left, right);
                printf(("[DEBUG] Changing Task to " + this->currentTask->toString() + "\n").c_str());
                ret = true;
            } else if (this->currentTask == NULL && this->tasks.empty()) {
                break;
            }

            if (this->currentTask != NULL && this->currentTask->update(left, right, this->taskTick)) {
                printf(("[DEBUG] Task " + this->currentTask->toString() + " finished!\n").c_str());
                delete this->currentTask;
                this->currentTask = NULL;
            } else {
                ret = true;
                break;
            }
        }

        this->taskTick++;
        xSemaphoreGive(this->taskSemaphore);

        return ret;
    }
};
