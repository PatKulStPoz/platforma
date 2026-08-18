#pragma once

#include "esp_wifi.h"

#define CONN_WIFI_ENABLED true
#define CONN_WIFI_SSID      "Platforma_Hotspot"
#define CONN_WIFI_PASS      "Platforma2026"
#define CONN_WIFI_MODE WIFI_AUTH_WPA2_PSK
#define CONN_WIFI_MAXIMUM_RETRY  5

#define AP_WIFI_ENABLED false
#define AP_WIFI_SSID      "mywifissid"
#define AP_WIFI_PASS      "mywifipass"

#define MDNS_HOSTNAME      "esp32"
#define MDNS_NAME      "Platforma Jezdzaca!"

#define EXAMPLE_ESP_WIFI_CHANNEL   1
#define EXAMPLE_MAX_STA_CONN       4

static const char *TAG = "wifi softAP";