#include "custom_wifi.h"
#include "xense.h"

#define NVS_STORAGE_NAMESPACE "xense"
#define NVS_WIFI_KEY "wifi_sta"
#define NVS_MAC_ESP_NOW_KEY "esp_now_mac"

esp_err_t save_station_config(const Xense_Station *station) {
  nvs_handle_t handle;
  esp_err_t err = nvs_open(NVS_STORAGE_NAMESPACE, NVS_READWRITE, &handle);
  if (err != ESP_OK) {
    return err;
  }

  err = nvs_set_blob(handle, NVS_WIFI_KEY, station, sizeof(Xense_Station));
  if (err == ESP_OK) {
    err = nvs_commit(handle);
  }

  nvs_close(handle);
  return err;
}

esp_err_t load_station_config(Xense_Station *station) {
  nvs_handle_t handle;
  esp_err_t err = nvs_open(NVS_STORAGE_NAMESPACE, NVS_READONLY, &handle);
  if (err != ESP_OK) {
    return err;
  }

  size_t size = sizeof(Xense_Station);
  err = nvs_get_blob(handle, NVS_WIFI_KEY, station, &size);

  nvs_close(handle);
  return err;
}

esp_err_t save_mac_esp_now_key(const uint8_t *mac) {
  nvs_handle_t handle;
  esp_err_t err = nvs_open(NVS_STORAGE_NAMESPACE, NVS_READWRITE, &handle);
  if (err != ESP_OK) {
    return err;
  }

  err = nvs_set_blob(handle, NVS_MAC_ESP_NOW_KEY, mac, 6);
  if (err != ESP_OK) {
  } else {
    err = nvs_commit(handle);
    if (err != ESP_OK) {
    }
  }

  nvs_close(handle);
  return err;
}

esp_err_t load_mac_esp_now_key(uint8_t *mac) {
  nvs_handle_t handle;
  esp_err_t err = nvs_open(NVS_STORAGE_NAMESPACE, NVS_READONLY, &handle);
  if (err != ESP_OK) {
    return err;
  }

  size_t size = 6;
  err = nvs_get_blob(handle, NVS_MAC_ESP_NOW_KEY, mac, &size);

  nvs_close(handle);
  return err;
}
