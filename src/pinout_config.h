#pragma once

#include "driver/gpio.h"
#include "driver/dac_oneshot.h"
#include "driver/ledc.h"

// https://randomnerdtutorials.com/esp32-pinout-reference-gpios/

#define STATUS_LED GPIO_NUM_14
#define STATUS_BATTERY_READ GPIO_NUM_36

typedef struct {
    dac_channel_t control_channel;
    gpio_num_t direction_out;
    gpio_num_t brake_out;
    ledc_channel_t brake_channel;
    gpio_num_t driver_in;
    gpio_num_t hall_back_in;
    gpio_num_t hall_main_in;
    gpio_num_t hall_front_in;
} DriverPinout;


// DAC_CHAN_0 = Pin 25
// DAC_CHAN_1 = Pin 26
//DriverPinout LEFT_DRIVER_PINS = {DAC_CHAN_0, GPIO_NUM_19, GPIO_NUM_21, GPIO_NUM_18, GPIO_NUM_27, GPIO_NUM_33, GPIO_NUM_13 };
//DriverPinout RIGHT_DRIVER_PINS = {DAC_CHAN_1, GPIO_NUM_5, GPIO_NUM_25, GPIO_NUM_22, GPIO_NUM_17, GPIO_NUM_4, GPIO_NUM_16 };


DriverPinout LEFT_DRIVER_PINS = {
    .control_channel = DAC_CHAN_0, 
    .direction_out = GPIO_NUM_13, 
    .brake_out = GPIO_NUM_12,
    .brake_channel = LEDC_CHANNEL_0,
    .driver_in = GPIO_NUM_27, 
    .hall_back_in = GPIO_NUM_34, 
    .hall_main_in = GPIO_NUM_35, 
    .hall_front_in = GPIO_NUM_32
};

DriverPinout RIGHT_DRIVER_PINS = {
    .control_channel = DAC_CHAN_1, 
    .direction_out = GPIO_NUM_15, 
    .brake_out = GPIO_NUM_2,
    .brake_channel = LEDC_CHANNEL_1,
    .driver_in = GPIO_NUM_4, 
    .hall_back_in = GPIO_NUM_5, 
    .hall_main_in = GPIO_NUM_17, 
    .hall_front_in = GPIO_NUM_16
};

// 16 / 22 (3 input only, 1 reg)