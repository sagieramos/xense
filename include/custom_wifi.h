#ifndef CUSTOM_WIFI_H
#define CUSTOM_WIFI_H

#include "config_xense.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "xense.h"

#define CUSTOM_WIFI_TAG "WiFiModule"
#define DEFAULT_SCAN_LIST_SIZE 20
#define NVS_STORAGE_NAMESPACE "xense"
#define NVS_WIFI_KEY "wifi_sta"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_SCAN_DONE_BIT BIT1
static EventGroupHandle_t wifi_event_group;

extern wifi_config_t ap_config;
extern wifi_scan_config_t scan_conf;

void set_xense_hostname(esp_netif_t *sta_netif);
void set_xense_ap_ssid(wifi_config_t *ap_config);

void wifi_init_sta();
void wifi_init_ap();
void wifi_init_ap_sta();
void wifi_scan();

esp_err_t save_station_config(const Xense_Station *station);
esp_err_t load_station_config(Xense_Station *station);

void wifi_scan_process_task(void *pvParameters);

#endif // CUSTOM_WIFI_H