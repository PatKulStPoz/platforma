#pragma once
#include "driver.h"
#include <vector>

enum CmdDriver {
    CMD_DRIVER_NONE = 0,
    CMD_DRIVER_LEFT = 1,
    CMD_DRIVER_RIGHT = 2,
    CMD_DRIVER_BOTH = 3
};

class CommandTask {
    public:
    virtual ~CommandTask() {};
    virtual void setup(Driver* left, Driver* right) {};
    // true -> zakończony, false -> nie
    virtual bool update(Driver* left, Driver* right, int tick) {
        return true;
    };
    virtual void reset() {};
};

class SetLevelTask : public CommandTask {
    CmdDriver driver;
    uint8_t level;
    public:
    SetLevelTask(uint8_t level) {
        this->driver = CMD_DRIVER_BOTH;
        this->level = level; 
    }

    SetLevelTask(CmdDriver driver, uint8_t level) {
        this->driver = driver;
        this->level = level;
    }
    virtual void setup(Driver* left, Driver* right) {
        if (this->driver & CMD_DRIVER_LEFT) left->setLevel(this->level);
        if (this->driver & CMD_DRIVER_RIGHT) right->setLevel(this->level);
    }
};

class SetDirectionTask : public CommandTask {
    CmdDriver driver;
    DriverDirection direction;
    public:
    SetDirectionTask(DriverDirection direction) {
        this->driver = CMD_DRIVER_BOTH;
        this->direction = direction; 
    }

    SetDirectionTask(CmdDriver driver, DriverDirection direction) {
        this->driver = driver;
        this->direction = direction; 
    }
    virtual void setup(Driver* left, Driver* right) {
        if (this->driver & CMD_DRIVER_LEFT) left->setDirection(this->direction);
        if (this->driver & CMD_DRIVER_RIGHT) right->setDirection(this->direction);
    }
};

class RotateTask : public CommandTask {
    CmdDriver driver;
    int degrees;
    public:
    RotateTask(int degrees) {
        this->driver = CMD_DRIVER_BOTH;
        this->degrees = degrees;
    }
    RotateTask(CmdDriver driver, int degrees) {
        this->driver = driver;
        this->degrees = degrees;
    }

    virtual void setup(Driver* left, Driver* right) {
        if (this->driver & CMD_DRIVER_LEFT) {
            left->rotateDeg(this->degrees);
            left->start();
        }
        if (this->driver & CMD_DRIVER_RIGHT) {
            right->rotateDeg(this->degrees);
            right->start();
        }
    }
    
    virtual bool update(Driver* left, Driver* right, int tick) {
        return true;
    }
};

class StartTask : public CommandTask {
    CmdDriver driver;
    public:
    StartTask() {
        this->driver = CMD_DRIVER_BOTH;
    }
    StartTask(CmdDriver driver) {
        this->driver = driver;
    }

    virtual void setup(Driver* left, Driver* right) {
        if (this->driver & CMD_DRIVER_LEFT) left->start();
        if (this->driver & CMD_DRIVER_RIGHT) right->start();
    }
    
    virtual bool update(Driver* left, Driver* right, int tick) {
        return true;
    }
};

class StopTask : public CommandTask {
    CmdDriver driver;
    public:
    StopTask() {
        this->driver = CMD_DRIVER_BOTH;
    }
    StopTask(CmdDriver driver) {
        this->driver = driver;
    }

    virtual void setup(Driver* left, Driver* right) {
        if (this->driver & CMD_DRIVER_LEFT) left->stop();
        if (this->driver & CMD_DRIVER_RIGHT) right->stop();
    }
    
    virtual bool update(Driver* left, Driver* right, int tick) {
        return true;
    }
};

class WaitForFinishedTask : public CommandTask {
    CmdDriver driver;
    public:
    WaitForFinishedTask() {
        this->driver = CMD_DRIVER_BOTH;
    }

    WaitForFinishedTask(CmdDriver driver) {
        this->driver = driver;
    }

    virtual bool update(Driver* left, Driver* right, int tick) {
        return (!(this->driver & CMD_DRIVER_LEFT) || !left->isAutomatic()) && (!(this->driver & CMD_DRIVER_RIGHT) || !right->isAutomatic());
    }
};

class WaitTicksTask : public CommandTask {
    int ticks;
    public:
    // tick -> ~10ms
    WaitTicksTask(int ticks) {
        this->ticks = ticks;
    }
    
    virtual bool update(Driver* left, Driver* right, int tick) {
        return tick >= this->ticks;
    }
};

class RepeatTasks : public CommandTask {
    std::vector<CommandTask*> tasks;

    int task = 0;
    CommandTask* currentTask = NULL;
    int loop = 0;
    int tick = 0;
    int repeats;

    public:
    RepeatTasks(std::initializer_list<CommandTask*> tasks, int repeats) {
        this->tasks = tasks;
        this->repeats = repeats;
    }

    virtual ~RepeatTasks() {
        for (CommandTask* task : this->tasks) {
            delete task;
        }
    };

    
    virtual bool update(Driver* left, Driver* right, int tick) {
        while(this->repeats > this->loop) {
            if (this->currentTask == NULL) {
                this->tick = 0;
                this->currentTask = tasks.at(this->task);
                this->currentTask->setup(left, right);
                printf("RepeatTasks: Changing Task\n");
            }

            if (this->currentTask->update(left, right, this->tick++)) {
                this->currentTask->reset();
                this->currentTask = NULL;
                printf("RepeatTasks: Finished Task\n");

                if (++this->task == this->tasks.size()) {
                    printf("RepeatTasks: Reset\n");
                    this->loop++;
                    this->task = 0;
                }

                if (this->loop == this->repeats) {
                    printf("RepeatTasks: Loops finished\n");
                    return true;
                }
            } else {
                return false;
            }
        }
        printf("RepeatTasks: Bad call?\n");

        return true;
    }

    virtual void reset() {
        for (CommandTask* task : this->tasks) {
            task->reset();
        }
        this->task = 0;
        this->currentTask = NULL;
        this->loop = 0;
    };
};