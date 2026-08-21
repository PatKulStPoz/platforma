#pragma once
#include "driver/gpio.h"
#include "driver/ledc.h"
#include <math.h>

#include <stdio.h>
#include "driver/ledc.h"
#include "esp_err.h"
#include "sdkconfig.h"
#include "esp_pm.h"

#define SERVO_TIMER              LEDC_TIMER_0


static void setup_servo() {
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .duty_resolution  = LEDC_TIMER_13_BIT,
        .timer_num        = SERVO_TIMER,
        .freq_hz          = 50,
        .clk_cfg          = LEDC_AUTO_CLK,
        .deconfigure = false
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));
}


class Servo {
    ledc_channel_t channel;
    gpio_num_t pin;

    public:
    Servo(const gpio_num_t pin, const ledc_channel_t channel) {
        this->channel = channel;
        this->pin = pin;
    }

    void setup() {
        ledc_channel_config_t ledc_channel = {
            .gpio_num       = this->pin,
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .channel = this->channel,
            .intr_type = LEDC_INTR_DISABLE,
            .timer_sel = SERVO_TIMER,
            .duty  = 8192 * (20 * 90 / 180 + 5) / 200,
            .hpoint = 0,
            .sleep_mode = LEDC_SLEEP_MODE_NO_ALIVE_NO_PD,
            .flags = {.output_invert = false},
            .deconfigure = false
        };
        ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
    }

    void destroy() {
        ledc_channel_config_t ledc_channel = {
            .gpio_num = this->pin,
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .channel = this->channel,
            .intr_type = LEDC_INTR_DISABLE,
            .timer_sel = SERVO_TIMER,
            .duty  = 0,
            .hpoint = 0,
            .sleep_mode = LEDC_SLEEP_MODE_INVALID,
            .flags = {.output_invert = false},
            .deconfigure = true
        };
        ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
    }

    void setAngle(uint8_t degrees) {
        // 0.5 ms - 2.5 ms.-> 2.5% - 12.5%
        if (degrees > 180) {
            degrees = 180;
        }

        ledc_set_duty(LEDC_LOW_SPEED_MODE, this->channel, 8192 * (20 * degrees / 180 + 5) / 200);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, this->channel);
    }
};
