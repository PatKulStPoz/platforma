#include "driver/gpio.h"


// https://docs.espressif.com/projects/esp-chip-errata/en/latest/esp32/03-errata-description/esp32/gpio-edge-interrupts.html

//gpio_int_type_t

#define FAUX_GPIO_INTR_POSEDGE GPIO_INTR_HIGH_LEVEL
#define FAUX_GPIO_INTR_NEGEDGE GPIO_INTR_LOW_LEVEL

gpio_int_type_t workaround_intr_active[GPIO_NUM_MAX];
gpio_int_type_t workaround_intr_state[GPIO_NUM_MAX];

void workaround_set_intr_type(gpio_num_t pin, gpio_int_type_t type) {
    workaround_intr_active[pin] = type;
    workaround_intr_state[pin] = type;
    gpio_set_intr_type(pin, type);
}

// true, jeśli nie powinien być osłużony
bool workaround_intr(gpio_num_t pin) {
    bool res = workaround_intr_state[pin] != workaround_intr_active[pin];
    gpio_int_type_t alt = workaround_intr_state[pin] == GPIO_INTR_HIGH_LEVEL ? GPIO_INTR_LOW_LEVEL : GPIO_INTR_HIGH_LEVEL;
    workaround_intr_state[pin] = alt;
    gpio_set_intr_type(pin, alt);
    return res;
}