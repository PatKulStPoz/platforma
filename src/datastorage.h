#include "nvs.h"
#include "nvs_flash.h"
#include "esp_log.h"

template <typename T>
esp_err_t save_data(const char* name, T* value) {
    static const char* TAG = "platforma-datastorage";

    nvs_handle_t handle;
    esp_err_t err;

    // Open NVS handle
    err = nvs_open("platforma", NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return err;
    }

    err = nvs_set_blob(handle, name, value, sizeof(T));

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write '%s' data (%d)!", name, err);
        nvs_close(handle);
        return err;
    }

    // Commit
    err = nvs_commit(handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to commit data");
    }

    nvs_close(handle);
    return err;
}

template <typename T>
esp_err_t load_data(const char* name, T* value) {
    static const char* TAG = "platforma-datastorage";

    nvs_handle_t handle;
    esp_err_t err;

    // Open NVS handle
    err = nvs_open("platforma", NVS_READONLY, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return err;
    }

    size_t size = sizeof(T);
    err = nvs_get_blob(handle, name, value, &size);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to load '%s' data %d!", name, err);
        nvs_close(handle);
        return err;
    }

    nvs_close(handle);
    return err;
}