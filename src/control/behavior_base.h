#pragma once
#include <string>

class Driver;

enum BehaviorType {
    BEHAVIOR_UNDEF,
    BEHAVIOR_DEFAULT,
    BEHAVIOR_ROTATION
};

class BaseBehavior {
    protected:
    Driver* driver;
    BaseBehavior* previousBehavior = NULL;

    public:
    bool running = false;
    virtual ~BaseBehavior();
    virtual void setup(Driver* driver, BaseBehavior* previousBehavior);
    virtual void update() {};
    virtual void onLevelSet(uint8_t level) {};
    virtual void onMainHallTick(uint32_t tick) {};
    virtual void onDriverTick(uint32_t tick) {};
    virtual void start() {}
    virtual void stop() {}
    virtual bool isAutomated() {
        return false;
    }

    virtual bool restorePrevious() {
        return false;
    }

    virtual void setTargetLevelForce(uint8_t level);

    virtual void setTargetLevel(uint8_t level);

    BaseBehavior* getPreviousBehavior(bool clear);

    virtual std::string toString() = 0;

    virtual std::string toStringWithExtra() {
        return this->toString() + " / running=" + std::to_string(this->isRunning()) + ", running_self=" + std::to_string(this->running);
    }

    virtual bool isRunning() {
        return this->running;
    }

    virtual BehaviorType type() {
        return BEHAVIOR_UNDEF;
    }
};

BaseBehavior* createDefaultBehavior();