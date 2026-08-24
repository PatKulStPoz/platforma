#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/soc_caps.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "pinout_config.h"



class AdcReader {
    adc_channel_t channel;
    adc_atten_t atten;
    adc_oneshot_unit_handle_t handle = NULL;
    adc_cali_handle_t caliHandle = NULL;
    bool do_calibration = false;

    public:
    AdcReader(adc_channel_t channel, adc_atten_t atten) {
        this->channel = channel;
        this->atten = atten;

        adc_oneshot_unit_init_cfg_t init_config = {
            .unit_id = ADC_UNIT_1,
            .clk_src = ADC_RTC_CLK_SRC_RC_FAST,
            .ulp_mode = ADC_ULP_MODE_DISABLE
        };
        ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &this->handle));

        adc_oneshot_chan_cfg_t config = {
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };

        ESP_ERROR_CHECK(adc_oneshot_config_channel(this->handle, STATUS_BATTERY_READ_CHANNEL, &config));

        this->do_calibration = adc_calibration_init(ADC_UNIT_1, STATUS_BATTERY_READ_CHANNEL, atten, &this->caliHandle);
    }

    ~AdcReader() {
        ESP_ERROR_CHECK(adc_oneshot_del_unit(handle));
        if (this->do_calibration) {
            example_adc_calibration_deinit(caliHandle);
        }
    }


    int readRaw() {
        int val = 0;
        ESP_ERROR_CHECK(adc_oneshot_read(this->handle, this->channel, &val));
        return val;
    }

    int readMilliVolt() {
        int val = 0;
        if (this->do_calibration) {
            ESP_ERROR_CHECK(adc_cali_raw_to_voltage(caliHandle, this->readRaw(), &val));
        }
        return val;
    }

    private:
    static bool adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle) {
        adc_cali_handle_t handle = NULL;
        esp_err_t ret = ESP_FAIL;
        bool calibrated = false;

        #if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
            if (!calibrated) {
                adc_cali_curve_fitting_config_t cali_config = {
                    .unit_id = unit,
                    .chan = channel,
                    .atten = atten,
                    .bitwidth = ADC_BITWIDTH_DEFAULT,
                };
                ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
                if (ret == ESP_OK) {
                    calibrated = true;
                }
            }
        #endif

        #if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
            if (!calibrated) {
                adc_cali_line_fitting_config_t cali_config = {
                    .unit_id = unit,
                    .atten = atten,
                    .bitwidth = ADC_BITWIDTH_DEFAULT,
                    .default_vref = 3300
                };
                ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
                if (ret == ESP_OK) {
                    calibrated = true;
                }
            }
        #endif

        *out_handle = handle;
        if (ret == ESP_OK) {
        } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
            ESP_LOGW("adc_calibration", "eFuse not burnt, skip software calibration");
        } else {
            ESP_LOGE("adc_calibration", "Invalid arg or no memory");
        }

        return calibrated;
    }

    static void example_adc_calibration_deinit(adc_cali_handle_t handle) {
        #if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
            ESP_ERROR_CHECK(adc_cali_delete_scheme_curve_fitting(handle));

        #elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
            ESP_ERROR_CHECK(adc_cali_delete_scheme_line_fitting(handle));
        #endif
    }
};