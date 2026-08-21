
#pragma once
#include "../pinout_config.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include <math.h>
#include "workaround.h"
#include "behavior_base.h"
#include "servo.h"
#include "freertos/FreeRTOS.h"

extern void intrDriverIn(void* args);
extern void intrHallBack(void* args);
extern void intrHallMain(void* args);
extern void intrHallFront(void* args);



enum DriverDirection {
    DRIVER_FORWARD = 1,
    DRIVER_BACKWARDS = -1
};

enum HallId {
    HALL_FRONT = 0,
    HALL_MAIN = 1,
    HALL_BACK = 2,
    HALL_NONE = -1
};

typedef struct {
    bool has_brake = false;
    uint16_t driver_ticks_per_full_rotation = 256;
    uint16_t hall_sensor_ticks_per_full_rotation = 9;
} DriverConfig;

class Driver {
    private:
    const DriverPinout pinout;
    DriverConfig config = {};

    SemaphoreHandle_t behaviorSemaphore = NULL;

    Servo* brake;
    bool brakeValue = false;

    int driverTicks = 0;
    int driverTicksSec = 0;
    int driverTicksPerHal = 0;
    int halTicks = 0;
    DriverDirection direction = DRIVER_FORWARD;
    int intrCount = 0;

    uint8_t value = 0;
    uint16_t currentVal = 0;
    uint16_t targetVal = 0;

    dac_oneshot_handle_t dacHandle;

    HallId lastHall = HALL_NONE;
    HallId hallDirection = HALL_NONE;
    HallId previousHallDirection = HALL_NONE;

    BaseBehavior* behavior = createDefaultBehavior();

    void updateOutputLevel(uint8_t value) {
        dac_oneshot_output_voltage(this->dacHandle, value);
    }

    public:
    Driver(const DriverPinout pinout): pinout(pinout) {
        this->brake = new Servo(pinout.brake_out, pinout.brake_channel);
        vSemaphoreCreateBinary( this->behaviorSemaphore );
    }
    ~Driver() {
        delete this->behavior;
        delete this->brake;

        vSemaphoreDelete(this->behaviorSemaphore);
    }

    inline DriverConfig& getConfig() {
        return this->config;
    }

    inline const DriverPinout& getPinout() {
        return this->pinout;
    }

    // Konfiguruje gpio i funkcjonalność użytą przez silnik / driver
    void setup() {
        gpio_input_enable(this->pinout.driver_in);
        gpio_input_enable(this->pinout.hall_main_in);
        gpio_input_enable(this->pinout.hall_back_in);
        gpio_input_enable(this->pinout.hall_front_in);

        gpio_pullup_en(this->pinout.hall_main_in);
        gpio_pullup_en(this->pinout.hall_back_in);
        gpio_pullup_en(this->pinout.hall_front_in);

        DriverImpl::workaround_set_intr_type(this->pinout.driver_in, FAUX_GPIO_INTR_POSEDGE);
        gpio_isr_handler_add(this->pinout.driver_in, intrDriverIn, this);
        gpio_intr_enable(this->pinout.driver_in);

        DriverImpl::workaround_set_intr_type(this->pinout.hall_main_in, FAUX_GPIO_INTR_NEGEDGE);
        gpio_isr_handler_add(this->pinout.hall_main_in, intrHallMain, this);
        gpio_intr_enable(this->pinout.hall_main_in);

        DriverImpl::workaround_set_intr_type(this->pinout.hall_back_in, FAUX_GPIO_INTR_NEGEDGE);
        gpio_isr_handler_add(this->pinout.hall_back_in, intrHallBack, this);
        gpio_intr_enable(this->pinout.hall_back_in);

        DriverImpl::workaround_set_intr_type(this->pinout.hall_front_in, FAUX_GPIO_INTR_NEGEDGE);
        gpio_isr_handler_add(this->pinout.hall_front_in, intrHallFront, this);
        gpio_intr_enable(this->pinout.hall_front_in);


        gpio_output_enable(this->pinout.direction_out);
        gpio_set_level(this->pinout.direction_out, 0);

        dac_oneshot_config_t dacConfig = {
            .chan_id = this->pinout.control_channel
        };

        dac_oneshot_new_channel(&dacConfig, &this->dacHandle);
        dac_oneshot_output_voltage(this->dacHandle, 0);

        this->brake->setup();
        this->setBrake(false);
    }


    // Czysci zmiany zrobione przez driver
    void destroy() {
        gpio_reset_pin(this->pinout.brake_out);
        gpio_reset_pin(this->pinout.direction_out);
        gpio_reset_pin(this->pinout.driver_in);
        gpio_reset_pin(this->pinout.hall_main_in);

        gpio_isr_handler_remove(this->pinout.driver_in);
        gpio_isr_handler_remove(this->pinout.hall_main_in);
        gpio_isr_handler_remove(this->pinout.hall_back_in);
        gpio_isr_handler_remove(this->pinout.hall_front_in);

        dac_oneshot_del_channel(this->dacHandle);
        this->dacHandle = NULL;

        this->brake->destroy();
        this->setBrake(0);
    }

    // Obsługa przerwania od pwm płytki sterującej
    inline void handleDriverIn() {
        this->driverTicks++;
        this->driverTicksSec++;
        if (xSemaphoreTake(this->behaviorSemaphore, 0)) {
            this->behavior->onDriverTick(this->driverTicks);
            BaseType_t higherPriorityTaskWoken = pdFALSE;
            xSemaphoreGiveFromISR(this->behaviorSemaphore, &higherPriorityTaskWoken);
        }
    }

    // Obsługa przerwania od czujnika halla
    inline void handleHallMain() {
        this->intrCount++;
        this->halTicks++;
        this->driverTicksPerHal = this->driverTicksSec;
        this->driverTicksSec = 0;
        if (this->lastHall == HALL_BACK) {
            this->hallDirection = HALL_FRONT;
        } else if (this->lastHall == HALL_FRONT) {
            this->hallDirection = HALL_BACK;
        } 

        this->lastHall = HALL_MAIN;

        if (xSemaphoreTake(this->behaviorSemaphore, 0)) {
            this->behavior->onMainHallTick(this->halTicks);
            BaseType_t higherPriorityTaskWoken = pdFALSE;
            xSemaphoreGiveFromISR(this->behaviorSemaphore, &higherPriorityTaskWoken);
        }
    }

    inline void handleHallFront() {
        this->intrCount++;
        this->lastHall = HALL_FRONT;
    }

    inline void handleHallBack() {
        this->intrCount++;
        this->lastHall = HALL_BACK;
    }


    // Ustawia moc drivera
    // od 0 (0V) do 255 (3.3V) 
    inline void setLevel(uint8_t value) {
        if (this->value != value) {
            this->value = value;

            this->behavior->onLevelSet(value);
        }
    }

    inline uint8_t getLevel() {
        return this->value;
    }

    // Rozpoczyna obrót
    void start() {
        this->behavior->start();
    }

    // Konczy obrót
    void stop() {
        this->behavior->stop();
    }

    void reset() {
        this->stop();
        this->setLevel(0);
        xSemaphoreTake(this->behaviorSemaphore, portMAX_DELAY);

        this->behavior->getPreviousBehavior(true);
        delete this->behavior;
        this->behavior = createDefaultBehavior();
        this->behavior->setup(this, NULL);
        
        xSemaphoreGive(this->behaviorSemaphore);
        this->setDirection(DRIVER_FORWARD);
    }

    void setDirection(DriverDirection direction) {
        if (this->pinout.direction_out == GPIO_NUM_NC) {
            printf("Direction not supported!");
            return;
        }
        this->direction = direction;
        gpio_set_level(this->pinout.direction_out, direction == DRIVER_BACKWARDS);
    }

    void pushBehavior(BaseBehavior* behavior) {
        xSemaphoreTake(this->behaviorSemaphore, portMAX_DELAY);
        behavior->setup(this, this->behavior);
        this->behavior = behavior;
        xSemaphoreGive(this->behaviorSemaphore);
    }

    void resetBehavior() {
        xSemaphoreTake(this->behaviorSemaphore, portMAX_DELAY);
        BaseBehavior* old = this->behavior;
        this->behavior = createDefaultBehavior();
        this->behavior->setup(this, NULL);
        delete old;
        xSemaphoreGive(this->behaviorSemaphore);
    }

    void popBehavior() {
        xSemaphoreTake(this->behaviorSemaphore, portMAX_DELAY);
        BaseBehavior* old = this->behavior;
        BaseBehavior* previous = old->getPreviousBehavior(true);
        this->behavior = previous != NULL ? previous : createDefaultBehavior();
        delete old;
        xSemaphoreGive(this->behaviorSemaphore);
    }

    SemaphoreHandle_t& getBehaviorSemaphore() {
        return this->behaviorSemaphore;
    }

    BaseBehavior* getBehavior() {
        return this->behavior;
    }

    std::string getBehaviorToStringWithExtraSafe() {
        xSemaphoreTake(this->behaviorSemaphore, portMAX_DELAY);
        std::string res = this->behavior->toStringWithExtra();
        xSemaphoreGive(this->behaviorSemaphore);
        return res;
    }

    inline void setBrake(bool value) {
        this->brake->setAngle(value ? 10 : 170);
        this->brakeValue = value;
    }

    inline bool getBrake() {
        return this->brakeValue;
    }

    inline void setTargetLevel(uint8_t value) {
        this->targetVal = value << 2;
    }

    inline void setTargetLevelForce(uint8_t value) {
        this->targetVal = value << 2;
        this->currentVal = this->targetVal;
        this->updateOutputLevel(value);
    }

    // Aktualizuje stan na pinach.
    void update() {
        if (this->behavior->restorePrevious()) {
            this->popBehavior();
        }
        this->behavior->update();

        if (this->currentVal < this->targetVal) {
            if (this->currentVal < (40 << 2)) {
                this->currentVal = (40 << 2);
                updateOutputLevel((this->currentVal) >> 2);
            } else {
                updateOutputLevel((++this->currentVal) >> 2);
            }
        } else if (this->currentVal > this->targetVal) {
            this->currentVal = this->targetVal;
            updateOutputLevel(--this->currentVal >> 2);
        }
    }


    // Tiknięcia od sterownika na 1 halla
    inline int getDriverTicksPerHal() {
        return this->driverTicksPerHal;
    }

    //gety
    inline int getHallTicks() {
        return this->halTicks;
    }

    inline int getDriverTicks() {
        return this->driverTicks;
    }

    inline int getIntrCount() {
        return this->intrCount;
    }

    inline HallId getLastHall() {
        return this->lastHall;
    }

    inline HallId getHallDirection() {
        return this->hallDirection;
    }


    inline bool isAutomatic() {
        return this->behavior->isAutomated();
    }

    // Zeruje liczniki liczące.
    void clearCount() {
        this->halTicks = 0;
        this->driverTicks = 0;
        this->driverTicksSec = 0;
        this->driverTicksPerHal = 0;
    }
};

void intrDriverIn(void* args) {
    if (DriverImpl::workaround_intr(static_cast<Driver*>(args)->getPinout().driver_in)) return;
    static_cast<Driver*>(args)->handleDriverIn();
}

void intrHallMain(void* args) {
    if (DriverImpl::workaround_intr(static_cast<Driver*>(args)->getPinout().hall_main_in)) return;
    static_cast<Driver*>(args)->handleHallMain();
}

void intrHallBack(void* args) {
    if (DriverImpl::workaround_intr(static_cast<Driver*>(args)->getPinout().hall_back_in)) return;
    static_cast<Driver*>(args)->handleHallBack();
}

void intrHallFront(void* args) {
    if (DriverImpl::workaround_intr(static_cast<Driver*>(args)->getPinout().hall_front_in)) return;
    static_cast<Driver*>(args)->handleHallFront();
}