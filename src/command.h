#pragma once
#include "driver.h"
#include <vector>

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
    uint8_t level;
    public:
    SetLevelTask(uint8_t level) {
        this->level = level;
    }
    virtual void setup(Driver* left, Driver* right) {
        left->setLevel(this->level);
        right->setLevel(this->level);
    }
};


class RotateTask : public CommandTask {
    int degrees;
    uint8_t level;
    public:
    RotateTask(int degrees, uint8_t level=0) {
        this->degrees = degrees;
        this->level = level;
    }
    virtual void setup(Driver* left, Driver* right) {
        left->rotateDeg(this->degrees);
        right->rotateDeg(this->degrees);
        if (this->level != 0) {
            left->setLevel(this->level);
            right->setLevel(this->level);
        }
        left->start();
        right->start();
    }
    
    virtual bool update(Driver* left, Driver* right, int tick) {
        return true;
    }
};

class WaitForFinishedTask : public CommandTask {
    public:
    WaitForFinishedTask() {
    }
    
    virtual bool update(Driver* left, Driver* right, int tick) {
        return !left->isAutomatic() && !right->isAutomatic();
    }
};

class WaitTicksTask : public CommandTask {
    int ticks;
    public:
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