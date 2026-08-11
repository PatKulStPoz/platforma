
#pragma once
#include "driver_config.h"
#include "driver/gpio.h"
#include "driver/ledc.h"

extern void driver_prepare() {
}

extern void intrDriverIn(void* args);
extern void intrHalIn(void* args);

class Driver {
    private:
    DriverConfig config;
    int driverTicksFullRotation;
    int halTicksFullRotation;

    int driverTicks = 0;
    int driverTicksSec = 0;
    int rotationTick = -1;
    int driverTicksPerHal = 0;
    int halTicks = 0;

    uint8_t value = 0;
    uint8_t currentVal = 0;
    uint8_t targetVal = 0;

    dac_oneshot_handle_t dacHandle;

    bool automatic = false;

    void setLevelNoUpdate(uint8_t value) {
        this->targetVal = value << 2;
    }

    void updateAutomatic() {
        if (this->automatic && this->rotationTick-- <= 0) {
            setLevelNoUpdate(0);
            this->automatic = false;
        }
    }

    public:
    Driver(DriverConfig config, int driverTicksFullRotation, int halTicksPerFullRotation) {
        this->config = config;
        this->driverTicksFullRotation = driverTicksFullRotation;
        this->halTicksFullRotation = halTicksPerFullRotation;
    }

    void setup() {
        gpio_input_enable(this->config.driver_in);
        gpio_input_enable(this->config.hal_move_in);
        
        gpio_set_intr_type(this->config.driver_in, GPIO_INTR_POSEDGE);
        gpio_isr_handler_add(this->config.driver_in, intrDriverIn, this);
        gpio_intr_enable(this->config.driver_in);

        gpio_set_intr_type(this->config.hal_move_in, GPIO_INTR_POSEDGE);
        gpio_isr_handler_add(this->config.hal_move_in, intrHalIn, this);
        gpio_intr_enable(this->config.hal_move_in);


        /*ledc_channel_config_t ledc_channel = {
            .gpio_num = this->config.control_out,
            .speed_mode = LEDC_HIGH_SPEED_MODE ,
            .channel = this->config.control_channel,
            .intr_type = LEDC_INTR_DISABLE,
            .timer_sel = LEDC_TIMER_0,
            .duty = 0,
            .hpoint = 0,
            .sleep_mode = LEDC_SLEEP_MODE_KEEP_ALIVE,
            .flags = {1},
            .deconfigure = false
        };

        ledc_channel_config(&ledc_channel);*/
        //gpio_output_enable(this->config.control_out);

        dac_oneshot_config_t dacConfig = {
            .chan_id = this->config.control_channel
        };

        dac_oneshot_new_channel(&dacConfig, &this->dacHandle);
    }

    void clear() {
        gpio_reset_pin(this->config.brakes_out);
        gpio_reset_pin(this->config.direction_out);
        gpio_reset_pin(this->config.driver_in);
        gpio_reset_pin(this->config.hal_move_in);

        gpio_isr_handler_remove(this->config.driver_in);
        gpio_isr_handler_remove(this->config.hal_move_in);

        dac_oneshot_del_channel(this->dacHandle);
        this->dacHandle = NULL;
    }

    void debug() {
        printf("Driver %d, Hal: %d\n", this->driverTicks, this->halTicks);
    }

    void handleDriverIn() {
        this->driverTicks++;
        this->driverTicksSec++;
    }

    void handleHalIn() {
        this->halTicks++;
        this->driverTicksPerHal = this->driverTicksSec;
        this->driverTicksSec = 0;
        this->updateAutomatic();

    }

    void rotateDeg(int deg) {
        if (this->config.hal_move_in == GPIO_NUM_NC) {
            return;
        } 

        gpio_intr_disable(this->config.hal_move_in);

        this->rotationTick = (deg - 1) * this->halTicksFullRotation / 360;
        this->automatic = true;

        printf("Rotate %d deg (%d) hal ticks\n", deg, this->rotationTick);

        gpio_intr_enable(this->config.hal_move_in);
    }

    void setLevel(uint8_t value) {
        if (this->value != value) {
            this->value = value;
        }
    }

    void start() {
        this->setLevelNoUpdate(this->value);
    }

    void stop() {
        this->setLevelNoUpdate(0);
    }

    void update() {
        if (this->currentVal < this->targetVal) {
            dac_oneshot_output_voltage(this->dacHandle, (++this->currentVal) >> 2);
        } else if (this->currentVal > this->targetVal) {
            this->currentVal = this->targetVal;
            dac_oneshot_output_voltage(this->dacHandle, this->currentVal >> 2);
        }

        //printf("Current: %d\n", this->driverTicks);
    }

    int getDriverTicksPerHal() {
        return this->driverTicksPerHal;
    }

    int getHalTicks() {
        return this->halTicks;
    }

    bool isAutomatic() {
        return this->automatic;
    }

    void clearCount() {
        this->halTicks = 0;
        this->driverTicks = 0;
        this->driverTicksSec = 0;
        this->driverTicksPerHal = 0;
    }
};

void intrDriverIn(void* args) {
    static_cast<Driver*>(args)->handleDriverIn();
}

void intrHalIn(void* args) {
    static_cast<Driver*>(args)->handleHalIn();
}