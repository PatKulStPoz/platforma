#pragma once
#include "driver.h"
#include <vector>
#include <string>
#include "behavior.h"

enum TaskedDriver {
    TASK_DRIVER_NONE = 0,
    TASK_DRIVER_LEFT = 1,
    TASK_DRIVER_RIGHT = 2,
    TASK_DRIVER_BOTH = 3
};

class BaseTask {
    public:
    virtual ~BaseTask() {};
    virtual void setup(Driver* left, Driver* right) {};
    // true -> zakończony, false -> nie
    virtual bool update(Driver* left, Driver* right, uint32_t tick) {
        return true;
    };
    virtual void reset() {};

    virtual std::string toString() = 0;
};

class SetLevelTask : public BaseTask {
    TaskedDriver driver;
    uint8_t level;
    public:
    SetLevelTask(uint8_t level) {
        this->driver = TASK_DRIVER_BOTH;
        this->level = level; 
    }

    SetLevelTask(TaskedDriver driver, uint8_t level) {
        this->driver = driver;
        this->level = level;
    }
    virtual void setup(Driver* left, Driver* right) {
        if (this->driver & TASK_DRIVER_LEFT) left->setLevel(this->level);
        if (this->driver & TASK_DRIVER_RIGHT) right->setLevel(this->level);
    }

    virtual std::string toString() {
        return "SetLevelTask[driver=" + std::to_string(this->driver) + ", level=" + std::to_string(this->level) + "]";
    }
};

class SetDirectionTask : public BaseTask {
    TaskedDriver driver;
    DriverDirection direction;
    public:
    SetDirectionTask(DriverDirection direction) {
        this->driver = TASK_DRIVER_BOTH;
        this->direction = direction; 
    }

    SetDirectionTask(TaskedDriver driver, DriverDirection direction) {
        this->driver = driver;
        this->direction = direction; 
    }
    virtual void setup(Driver* left, Driver* right) {
        if (this->driver & TASK_DRIVER_LEFT) left->setDirection(this->direction);
        if (this->driver & TASK_DRIVER_RIGHT) right->setDirection(this->direction);
    }

    virtual std::string toString() {
        return "SetDirectionTask[driver=" + std::to_string(this->driver) + ", direction=" + std::to_string(this->direction) + "]";
    }
};

class RotateTask : public BaseTask {
    TaskedDriver driver;
    int degrees;
    public:
    RotateTask(int degrees) {
        this->driver = TASK_DRIVER_BOTH;
        this->degrees = degrees;
    }
    RotateTask(TaskedDriver driver, int degrees) {
        this->driver = driver;
        this->degrees = degrees;
    }

    virtual void setup(Driver* left, Driver* right) {
        if (this->driver & TASK_DRIVER_LEFT) {
            left->pushBehavior(this->driver & TASK_DRIVER_RIGHT ? new SyncedRotateBehavior(this->degrees, right) : new RotateBehavior(this->degrees));
            left->start();
        }
        if (this->driver & TASK_DRIVER_RIGHT) {
            left->pushBehavior(this->driver & TASK_DRIVER_LEFT ? new SyncedRotateBehavior(this->degrees, left) : new RotateBehavior(this->degrees));
            right->start();
        }
    }
    
    virtual bool update(Driver* left, Driver* right, uint32_t tick) {
        return true;
    }

    virtual std::string toString() {
        return "RotateTask[driver=" + std::to_string(this->driver) + ", degrees=" + std::to_string(this->degrees) + "]";
    }
};


class SetSyncBehaviorTask : public BaseTask {
    TaskedDriver driver;
    public:
    SetSyncBehaviorTask(TaskedDriver driver) {
        this->driver = driver;
    }

    SetSyncBehaviorTask() {
        this->driver = TASK_DRIVER_BOTH;
    }

    virtual void setup(Driver* left, Driver* right) {
        if (this->driver & TASK_DRIVER_LEFT) {
            left->resetBehavior();
            left->pushBehavior(new SyncedForwardBehavior(right));
        }
        if (this->driver & TASK_DRIVER_RIGHT) {
            right->resetBehavior();
            right->pushBehavior(new SyncedForwardBehavior(left));
        }
    }
    
    virtual bool update(Driver* left, Driver* right, uint32_t tick) {
        return true;
    }

    virtual std::string toString() {
        return "SetSyncBehaviorTask[driver=" + std::to_string(this->driver) + "]";
    }
};

class PopBehaviorTask : public BaseTask {
    TaskedDriver driver;
    public:
    PopBehaviorTask(TaskedDriver driver) {
        this->driver = driver;
    }
    PopBehaviorTask() {
        this->driver = TASK_DRIVER_BOTH;
    }

    virtual void setup(Driver* left, Driver* right) {
        if (this->driver & TASK_DRIVER_LEFT) {
            left->popBehavior();
        }
        if (this->driver & TASK_DRIVER_RIGHT) {
            right->popBehavior();
        }
    }
    
    virtual bool update(Driver* left, Driver* right, uint32_t tick) {
        return true;
    }

    virtual std::string toString() {
        return "PopBehaviorTask[driver=" + std::to_string(this->driver) + "]";
    }
};

class ResetBehaviorTask : public BaseTask {
    TaskedDriver driver;
    public:
    ResetBehaviorTask(TaskedDriver driver) {
        this->driver = driver;
    }
    ResetBehaviorTask() {
        this->driver = TASK_DRIVER_BOTH;
    }

    virtual void setup(Driver* left, Driver* right) {
        if (this->driver & TASK_DRIVER_LEFT) {
            left->resetBehavior();
        }
        if (this->driver & TASK_DRIVER_RIGHT) {
            right->resetBehavior();
        }
    }
    
    virtual bool update(Driver* left, Driver* right, uint32_t tick) {
        return true;
    }

    virtual std::string toString() {
        return "ResetBehaviorTask[driver=" + std::to_string(this->driver) + "]";
    }
};


class StartTask : public BaseTask {
    TaskedDriver driver;
    public:
    StartTask() {
        this->driver = TASK_DRIVER_BOTH;
    }
    StartTask(TaskedDriver driver) {
        this->driver = driver;
    }

    virtual void setup(Driver* left, Driver* right) {
        if (this->driver & TASK_DRIVER_LEFT) left->start();
        if (this->driver & TASK_DRIVER_RIGHT) right->start();
    }
    
    virtual bool update(Driver* left, Driver* right, uint32_t tick) {
        return true;
    }

    virtual std::string toString() {
        return "StartTask[driver=" + std::to_string(this->driver) + "]";
    }
};

class StopTask : public BaseTask {
    TaskedDriver driver;
    public:
    StopTask() {
        this->driver = TASK_DRIVER_BOTH;
    }
    StopTask(TaskedDriver driver) {
        this->driver = driver;
    }

    virtual void setup(Driver* left, Driver* right) {
        if (this->driver & TASK_DRIVER_LEFT) left->stop();
        if (this->driver & TASK_DRIVER_RIGHT) right->stop();
    }
    
    virtual bool update(Driver* left, Driver* right, uint32_t tick) {
        return true;
    }

    virtual std::string toString() {
        return "StopTask[driver=" + std::to_string(this->driver) + "]";
    }
};

class ResetTask : public BaseTask {
    TaskedDriver driver;
    public:
    ResetTask() {
        this->driver = TASK_DRIVER_BOTH;
    }

    ResetTask(TaskedDriver driver) {
        this->driver = driver;
    }

    virtual void setup(Driver* left, Driver* right) {
        if (this->driver & TASK_DRIVER_LEFT) left->reset();
        if (this->driver & TASK_DRIVER_RIGHT) right->reset();
    }

    virtual std::string toString() {
        return "ResetTask[driver=" + std::to_string(this->driver) + "]";
    }
};

class WaitForFinishedTask : public BaseTask {
    TaskedDriver driver;
    public:
    WaitForFinishedTask() {
        this->driver = TASK_DRIVER_BOTH;
    }

    WaitForFinishedTask(TaskedDriver driver) {
        this->driver = driver;
    }

    virtual bool update(Driver* left, Driver* right, uint32_t tick) {
        if (this->driver == TASK_DRIVER_NONE) return !right->isAutomatic() || !left->isAutomatic(); 

        return (!(this->driver & TASK_DRIVER_LEFT) || !left->isAutomatic()) && (!(this->driver & TASK_DRIVER_RIGHT) || !right->isAutomatic());
    }

    virtual std::string toString() {
        return "WaitForFinishedTask[driver=" + std::to_string(this->driver) + "]";
    }
};

class WaitTicksTask : public BaseTask {
    uint32_t ticks;
    public:
    // tick -> ~10ms
    WaitTicksTask(uint32_t ticks) {
        this->ticks = ticks;
    }
    
    virtual bool update(Driver* left, Driver* right, uint32_t tick) {
        return tick >= this->ticks;
    }

    virtual std::string toString() {
        return "WaitTicksTask[ticks=" + std::to_string(this->ticks) + "]";
    }
};

class RepeatTasks : public BaseTask {
    std::vector<BaseTask*> tasks;

    int task = 0;
    BaseTask* currentTask = NULL;
    int loop = 0;
    uint32_t tick = 0;
    int repeats;

    public:
    RepeatTasks(std::initializer_list<BaseTask*> tasks, int repeats) {
        this->tasks = tasks;
        this->repeats = repeats;
    }

    virtual ~RepeatTasks() {
        for (BaseTask* task : this->tasks) {
            delete task;
        }
    };

    
    virtual bool update(Driver* left, Driver* right, uint32_t tick) {
        while(this->repeats > this->loop) {
            if (this->currentTask == NULL) {
                this->tick = 0;
                this->currentTask = tasks.at(this->task);
                this->currentTask->setup(left, right);
                printf(("RepeatTasks: Changing Task to " + this->currentTask->toString() + "\n").c_str());
            }

            if (this->currentTask->update(left, right, this->tick++)) {
                printf(("RepeatTasks: Finished Task " + this->currentTask->toString() + "\n").c_str());
                this->currentTask->reset();
                this->currentTask = NULL;

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
        for (BaseTask* task : this->tasks) {
            task->reset();
        }
        this->task = 0;
        this->currentTask = NULL;
        this->loop = 0;
    };

    virtual std::string toString() {
        return "RepeatTask[tasks=[{" + std::to_string(this->tasks.size()) + "}], repeats=" + std::to_string(this->repeats) + "]";
    }
};