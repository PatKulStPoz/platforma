#pragma once

#include "driver/gpio.h"
#include "driver/dac_oneshot.h"
#include "driver/ledc.h"

typedef struct {
    dac_channel_t control_channel;
    gpio_num_t direction_out;
    gpio_num_t brakes_out;
    gpio_num_t driver_in;
    gpio_num_t hall_back_in;
    gpio_num_t hall_main_in;
    gpio_num_t hall_front_in;
} DriverConfig;


// DAC_CHAN_0 = Pin 25
// DAC_CHAN_1 = Pin 26
DriverConfig LEFT_DRIVER_PINS = {DAC_CHAN_0, GPIO_NUM_19, GPIO_NUM_NC, GPIO_NUM_18, GPIO_NUM_27, GPIO_NUM_33, GPIO_NUM_13 };
DriverConfig RIGHT_DRIVER_PINS = {DAC_CHAN_1, GPIO_NUM_5, GPIO_NUM_NC, GPIO_NUM_22, GPIO_NUM_17, GPIO_NUM_4, GPIO_NUM_16 };