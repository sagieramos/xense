#ifndef ESP_NOW_H
#define ESP_NOW_H

#include "esp_mac.h"
#include <cstring>
#include <esp_now.h>
#include <esp_wifi.h>

#define MAX_PEERS 20
#define MAX_BINARY_SIZE 128

#define ESP_NOW_TAG "ESP-NOW"

typedef struct {
  bool used;
  uint8_t mac[ESP_NOW_ETH_ALEN];
  uint8_t payload[MAX_BINARY_SIZE];
  size_t payload_len;
} peer_entry_t;

#endif // ESP_NOW_H