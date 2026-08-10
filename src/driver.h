
#pragma once
#include "driver_config.h"
#include "driver/gpio.h"
#include "driver/ledc.h"


ledc_timer_config_t ledc_timer = {
    .speed_mode       = LEDC_HIGH_SPEED_MODE ,
    .duty_resolution  = LEDC_TIMER_13_BIT,
    .timer_num        = LEDC_TIMER_0,
    .freq_hz          = 6000,  // Set output frequency at 4 kHz
    .clk_cfg          = LEDC_AUTO_CLK,
    .deconfigure = false,
};

extern void driver_prepare() {
    ledc_timer_config(&ledc_timer);
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

    dac_oneshot_handle_t dacHandle;

    bool automatic = false;

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

    void updateAutomatic() {
        if (this->automatic && this->rotationTick-- <= 0) {
            setLevel(0);
            this->automatic = false;
        }
    }

    void rotateDeg(int deg) {
        gpio_intr_disable(this->config.driver_in);

        this->rotationTick = (deg - 1) * this->halTicksFullRotation / 360;
        this->automatic = true;

        printf("Set: %d\n", this->rotationTick);

        gpio_intr_enable(this->config.driver_in);
    }

    void setLevel(uint8_t value) {
        if (this->value != value) {
            dac_oneshot_output_voltage(this->dacHandle, value);
            this->value = value;
        }
    }

    void update() {
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