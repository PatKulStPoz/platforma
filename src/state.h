#pragma once

#include "control/driver.h"
#include "control/tasks.h"
#include <queue>
#include "pinout_config.h"


typedef struct {
    char wifi_ssid[64] = "";
    char wifi_password[64] = "";
    bool wifi_ap = false;
    char hostname[64] = "esp32";
} PlatformConfig;

class TaskHandler {
    std::queue<BaseTask*> tasks;
    BaseTask* currentTask = NULL;
    SemaphoreHandle_t taskSemaphore = NULL;
    uint32_t taskTick = 0;
    
    public:
    TaskHandler() {
        vSemaphoreCreateBinary( this->taskSemaphore );
    }
    ~TaskHandler() {
        this->clearTasks();
        vSemaphoreDelete(this->taskSemaphore);
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

    bool tickTasks(Driver* left, Driver* right) {
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

class DriverState {
    Driver* left;
    Driver* right;

    PlatformConfig config;
    TaskHandler* tasks;
    uint32_t batteryVoltage = (BATTERY_MAX_VOLTAGE + BATTERY_MAX_VOLTAGE) / 2;

    public:
    DriverState(PlatformConfig config) {
        this->config = config;
        this->left = new Driver(LEFT_DRIVER_PINS);
        this->right = new Driver(RIGHT_DRIVER_PINS);
        this->left->setup();
        this->right->setup();
        this->tasks = new TaskHandler();
    }

    ~DriverState() {
        this->left->destroy();
        this->right->destroy();

        delete this->left;
        delete this->right;
        delete this->tasks;
    }

    PlatformConfig& getConfig() {
        return this->config;
    }

    Driver* leftDriver() {
        return this->left;
    }

    Driver* rightDriver() {
        return this->right;
    }

    TaskHandler* getTasks() {
        return this->tasks;
    }

    void setBatteryVoltage(uint32_t voltage) {
        this->batteryVoltage = voltage;
    }

    uint32_t getBatteryVoltage() {
        return this->batteryVoltage;
    }

    uint8_t getBatteryPercentage() {
        if (this->batteryVoltage < BATTERY_MIN_VOLTAGE) {
            return 0;
        }

        if (this->batteryVoltage > BATTERY_MAX_VOLTAGE) {
            return 100;
        }

        return 100 * (this->batteryVoltage - BATTERY_MIN_VOLTAGE) / (BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE);
    }

    bool tickTasks() {
        return this->tasks->tickTasks(this->left, this->right);
    }
};
