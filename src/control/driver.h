
#pragma once
#include "driver_config.h"
#include "driver/gpio.h"
#include "driver/ledc.h"

extern void driver_prepare() {
}

extern void intrDriverIn(void* args);
extern void intrHallBack(void* args);
extern void intrHallMain(void* args);
extern void intrHallFront(void* args);

enum DriverDirection {
    DRIVER_FORWARD = 1,
    DRIVER_BACKWARDS = -1
};

enum HallId {
    HALL_BACK = 2,
    HALL_MAIN = 1,
    HALL_FRONT = 0,
    HALL_NONE = -1
};

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
    DriverDirection direction = DRIVER_FORWARD;

    uint8_t value = 0;
    uint16_t currentVal = 0;
    uint16_t targetVal = 0;

    dac_oneshot_handle_t dacHandle;

    uint8_t hallOrderId = 0;
    HallId hallOrder[3] = {HALL_NONE, HALL_NONE, HALL_NONE};

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


    // Konfiguruje gpio i funkcjonalność użytą przez silnik / driver
    void setup() {
        gpio_input_enable(this->config.driver_in);
        gpio_input_enable(this->config.hall_main_in);
        
        gpio_set_intr_type(this->config.driver_in, GPIO_INTR_POSEDGE);
        gpio_isr_handler_add(this->config.driver_in, intrDriverIn, this);
        gpio_intr_enable(this->config.driver_in);

        gpio_set_intr_type(this->config.hall_main_in, GPIO_INTR_POSEDGE);
        gpio_isr_handler_add(this->config.hall_main_in, intrHallMain, this);
        gpio_intr_enable(this->config.hall_main_in);

        gpio_set_intr_type(this->config.hall_back_in, GPIO_INTR_POSEDGE);
        gpio_isr_handler_add(this->config.hall_back_in, intrHallBack, this);
        gpio_intr_enable(this->config.hall_back_in);

        gpio_set_intr_type(this->config.hall_front_in, GPIO_INTR_POSEDGE);
        gpio_isr_handler_add(this->config.hall_front_in, intrHallFront, this);
        gpio_intr_enable(this->config.hall_front_in);


        gpio_output_enable(this->config.direction_out);

        dac_oneshot_config_t dacConfig = {
            .chan_id = this->config.control_channel
        };

        dac_oneshot_new_channel(&dacConfig, &this->dacHandle);
    }


    // Czysci zmiany zrobione przez driver
    void destroy() {
        gpio_reset_pin(this->config.brakes_out);
        gpio_reset_pin(this->config.direction_out);
        gpio_reset_pin(this->config.driver_in);
        gpio_reset_pin(this->config.hall_main_in);

        gpio_isr_handler_remove(this->config.driver_in);
        gpio_isr_handler_remove(this->config.hall_main_in);
        gpio_isr_handler_remove(this->config.hall_back_in);
        gpio_isr_handler_remove(this->config.hall_front_in);

        dac_oneshot_del_channel(this->dacHandle);
        this->dacHandle = NULL;
    }

    void debug() {
        printf("Driver %d, Hal: %d\n", this->driverTicks, this->halTicks);
    }

    // Obsługa przerwania od pwm płytki sterującej
    void handleDriverIn() {
        this->driverTicks++;
        this->driverTicksSec++;
    }

    // Obsługa przerwania od czujnika halla
    void handleHallMain() {
        this->halTicks++;
        this->driverTicksPerHal = this->driverTicksSec;
        this->driverTicksSec = 0;
        this->updateAutomatic();
        this->hallOrder[ this->hallOrderId++ ] = HALL_MAIN;
        if (this->hallOrderId >= 3) this->hallOrderId = 0;
    }

    void handleHallFront() {
        this->hallOrder[ this->hallOrderId++ ] = HALL_FRONT;
        if (this->hallOrderId >= 3) this->hallOrderId = 0;
    }

    void handleHallBack() {
        this->hallOrder[ this->hallOrderId++ ] = HALL_BACK;
        if (this->hallOrderId >= 3) this->hallOrderId = 0;
    }

    // Obraca o "deg" stopni
    void rotateDeg(int deg) {
        if (this->config.hall_main_in == GPIO_NUM_NC) {
            return;
        } 
        gpio_intr_disable(this->config.hall_main_in);

        this->rotationTick = (deg - 1) * this->halTicksFullRotation / 360;
        this->automatic = true;

        printf("Rotate %d deg (%d) hal ticks\n", deg, this->rotationTick);

        gpio_intr_enable(this->config.hall_main_in);
    }


    // Ustawia moc drivera
    // od 0 (0V) do 255 (3.3V) 
    void setLevel(uint8_t value) {
        if (this->value != value) {
            this->value = value;
        }
    }

    // Rozpoczyna obrót
    void start() {
        this->setLevelNoUpdate(this->value);
    }

    // Konczy obrót
    void stop() {
        this->setLevelNoUpdate(0);
    }

    void setDirection(DriverDirection direction) {
        if (this->config.direction_out == GPIO_NUM_NC) {
            printf("Direction not supported!");
            return;
        }
        this->direction = direction;
        gpio_set_level(this->config.direction_out, direction == DRIVER_BACKWARDS);
    }


    // Aktualizuje stan na pinach.
    void update() {
        if (this->currentVal < this->targetVal) {
            dac_oneshot_output_voltage(this->dacHandle, (++this->currentVal) >> 2);
        } else if (this->currentVal > this->targetVal) {
            this->currentVal = this->targetVal;
            dac_oneshot_output_voltage(this->dacHandle, this->currentVal >> 2);
        }

        //printf("Current: %d\n", this->driverTicks);
    }


    // Tiknięcia od sterownika na 1 halla
    int getDriverTicksPerHal() {
        return this->driverTicksPerHal;
    }

    //gety
    int getHalTicks() {
        return this->halTicks;
    }

    int getDriverTicks() {
        return this->driverTicks;
    }

    int getRotationTick() {
        return this->rotationTick;
    }

    int getDriverTicksFullRotation() {
        return this->driverTicksFullRotation;
    }

    int getHalTicksFullRotation() {
        return this->halTicksFullRotation;
    }

    HallId getFirstHall() {
        return this->hallOrder[0];
    }

    HallId getLastHall() {
        return this->hallOrder[2];
    }

    bool isAutomatic() {
        return this->automatic;
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
    static_cast<Driver*>(args)->handleDriverIn();
}

void intrHallMain(void* args) {
    static_cast<Driver*>(args)->handleHallMain();
}

void intrHallBack(void* args) {
    static_cast<Driver*>(args)->handleHallBack();
}

void intrHallFront(void* args) {
    static_cast<Driver*>(args)->handleHallFront();
}