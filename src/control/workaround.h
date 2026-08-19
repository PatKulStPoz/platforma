#pragma once
#include "driver/gpio.h"


// https://docs.espressif.com/projects/esp-chip-errata/en/latest/esp32/03-errata-description/esp32/gpio-edge-interrupts.html

//gpio_int_type_t

#define FAUX_GPIO_INTR_POSEDGE GPIO_INTR_HIGH_LEVEL
#define FAUX_GPIO_INTR_NEGEDGE GPIO_INTR_LOW_LEVEL

namespace DriverImpl {

void workaround_set_intr_type(gpio_num_t pin, gpio_int_type_t type);

// true, jeśli nie powinien być osłużony
bool workaround_intr(gpio_num_t pin);

}