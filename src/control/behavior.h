#pragma once
#include "driver.h"
#include <vector>
#include <string>

BaseBehavior :: ~BaseBehavior() {
    if (this->previousBehavior != NULL) {
        delete this->previousBehavior;
    }
};

void BaseBehavior :: setup(Driver* driver, BaseBehavior* previousBehavior) {
    this->driver = driver;
    this->previousBehavior = previousBehavior;
};
   
void BaseBehavior :: setTargetLevelForce(uint8_t level) {
    this->driver->setTargetLevelForce(level);
}

void BaseBehavior :: setTargetLevel(uint8_t level) {
    this->driver->setTargetLevel(level);
}

BaseBehavior* BaseBehavior :: getPreviousBehavior(bool clear) {
    BaseBehavior* prev = this->previousBehavior;
    if (clear) {
        this->previousBehavior = NULL;
    }
    return prev;
}

class DefaultBehavior : public BaseBehavior {
    public:
    DefaultBehavior() {}

    virtual void start() {
        this->driver->setTargetLevel(this->driver->getLevel());
        this->running = true;
    }
    
    virtual void stop() {
        this->driver->setTargetLevelForce(0);
        this->running = false;
    }

    virtual void onSetLevel(uint8_t level) {
        this->setTargetLevel(level);
    }

    virtual BehaviorType type() {
        return BEHAVIOR_DEFAULT;
    }

    virtual std::string toString() {
        return "DefaultBehavior[]";
    }
};

BaseBehavior* createDefaultBehavior() {
    return new DefaultBehavior();
}

class RotateBehavior : public BaseBehavior {
    public:
    int32_t degrees;
    int32_t rotationTick;
    int32_t driverRotationTick;
    int32_t driverRotationTickLast;

    public:
    RotateBehavior(int degrees) {
        this->degrees = degrees;
    }

    virtual void setup(Driver* driver, BaseBehavior* previousBehavior) {
        BaseBehavior::setup(driver, previousBehavior);
        this->rotationTick = (this->degrees - 1) * driver->getConfig().hall_sensor_ticks_per_full_rotation / 360;
        this->driverRotationTick = (this->degrees - 1) * driver->getConfig().driver_ticks_per_full_rotation / 360;
        this->driverRotationTickLast = this->driverRotationTick;
    }

    virtual void onMainHallTick(uint32_t tick) {
        this->updateTargetSpeed();
            

        this->rotationTick--;
        this->driverRotationTickLast = this->driverRotationTick;
    }

    void updateTargetSpeed() {
        uint8_t value = this->driver->getLevel();

        if (!this->isRunning()) {
            this->setTargetLevelForce(0);
        } else if (this->rotationTick <= 0) {
            this->setTargetLevelForce(0);
            this->running = false;
        } else if (this->rotationTick <= 5) {
            this->setTargetLevel(value > 80 ? (value / 2 > 80 ? value : 80 ) : value);
        } else {
            this->setTargetLevel(value);
        }
    }

    virtual bool restorePrevious() {
        return this->rotationTick <= 0;
    }

    virtual bool isAutomated() {
        return true;
    }

    virtual void onDriverTick(uint32_t tick) {
        this->driverRotationTick--;
    }

    virtual void start() {
        this->running = true;
        this->updateTargetSpeed();
    }

    virtual void stop() {
        this->running = false;
        this->updateTargetSpeed();
    }

    virtual BehaviorType type() {
        return BEHAVIOR_ROTATION;
    }

    virtual std::string toString() {
        return "RotateBehavior[degrees=" + std::to_string(this->degrees) + ", rotationTick=" + std::to_string(this->rotationTick) + " ]";
    }
};

class SyncedRotateBehavior : public RotateBehavior {
    Driver* otherDriver;
    
    public:
    SyncedRotateBehavior(int degrees, Driver* otherDriver) : RotateBehavior(degrees) {
        this->otherDriver = otherDriver;
    }

    virtual void setTargetLevel(uint8_t level) {
        if (this->otherDriver->getBehavior()->type() == BEHAVIOR_ROTATION) {
            RotateBehavior* v = static_cast<RotateBehavior*>(this->otherDriver->getBehavior());
            int32_t val = level + (this->rotationTick - v->rotationTick) * 64;
            BaseBehavior::setTargetLevel(val > 255 ? 255 : (val < 0 ? 0 : val));
        } else {
            BaseBehavior::setTargetLevel(level);
        }
    }

    virtual bool isRunning() {
        return this->otherDriver->getBehavior()->running && this->running;
    }

    virtual std::string toString() {
        return "SyncedRotateBehavior[degrees=" + std::to_string(this->degrees) + ", rotationTick=" + std::to_string(this->rotationTick) + " ]";
    }
};

class SyncedForwardBehavior : public DefaultBehavior {
    Driver* otherDriver;
    uint32_t otherHallTickTarget;
    
    public:
    SyncedForwardBehavior(Driver* otherDriver) {
        this->otherDriver = otherDriver;
        this->otherHallTickTarget = otherDriver->getHallTicks();
    }

    virtual void setTargetLevel(uint8_t level) {
        if (this->otherDriver->getBehavior()->running) {
            int32_t val = level + (this->otherHallTickTarget - this->otherDriver->getHallTicks()) * 64;
            BaseBehavior::setTargetLevel(val > 255 ? 255 : (val < 0 ? 0 : val));
        } else {
            this->setTargetLevelForce(0);
        }
    }

    virtual void update() {
        this->setTargetLevel(this->driver->getLevel());
    }

    virtual void onMainHallTick(uint32_t) {
        this->otherHallTickTarget++;
    }

    virtual std::string toString() {
        return "SyncedForwardBehavior[]";
    }
};


