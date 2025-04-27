#include "xense.h"
#include "xense_utils.h"

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
