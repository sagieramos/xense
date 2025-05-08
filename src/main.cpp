#include "custom_wifi.h"
#include "driver/gpio.h"
#include "esp_event.h"
#include "esp_http_server.h"
#include "esp_mac.h"
#include "esp_netif.h"
#include "esp_random.h"
#include "esp_spiffs.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "mac_table.h"
#include "nvs_flash.h"
#include "server.h"
#include "spi_flash_mmap.h"
#include "xense_utils.h"
#include <lwip/api.h>
#include <lwip/netdb.h>
#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <stdio.h>
#include <stdlib.h>

// === APP MAIN ===
extern "C" void app_main(void);

void my_task(void *pvParameters) {
  vTaskDelay(pdMS_TO_TICKS(2000));
  // Run once
  // do_something();
  // Done!
  vTaskDelete(NULL);
}

static const char *TAG = "MAC_TABLE_TEST";

static MacTable table; // Global table declaration

static void handle_mac_event(size_t slot_index, MacTableResult status) {
  char mac_str[18];
  if (status == MAC_TABLE_TIMEOUT) {
    mac_to_str(table.entries[slot_index].mac, mac_str);
    ESP_LOGI(TAG, "Slot %u: MAC %s has expired", (unsigned)slot_index, mac_str);
  } else if (status == MAC_TABLE_INSERTED) {
    mac_to_str(table.entries[slot_index].mac, mac_str);
    ESP_LOGI(TAG, "Slot %u: MAC %s inserted", (unsigned)slot_index, mac_str);
  } else if (status == MAC_TABLE_UPDATED) {
    mac_to_str(table.entries[slot_index].mac, mac_str);
    ESP_LOGI(TAG, "Slot %u: MAC %s updated", (unsigned)slot_index, mac_str);
  } else if (status == MAC_TABLE_DELETED) {
    ESP_LOGI(TAG, "Slot %u: Entry deleted", (unsigned)slot_index);
  } else if (status == MAC_TABLE_FULL) {
    ESP_LOGI(TAG, "Slot %u: Mac table is full", (unsigned)slot_index);
  } else {
    ESP_LOGI(TAG, "Slot %u: Status %d", (unsigned)slot_index, status);
  }
}

void app_main(void) {
  init_led_cmd();
  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  wifi_init_ap_sta();
  xTaskCreate(wifi_scan_process_task, "wifi_scan_process_task", 4096, NULL, 5,
              NULL);

  ESP_LOGI("MAIN", "ESP32 ESP-IDF WebSocket Web Server is running ... ...\n");
  init_web_page_buffers();
  setup_server();

  ESP_LOGI(TAG, "Starting MAC table test");

  MacTableEntry entries[10];

  // Initialize MAC table with 5-second expiration for testing
  mac_table_init(&table, entries, 5, 60, handle_mac_event);

  // Test 1: Insert a MAC address
  uint8_t mac1[MAC_ADDR_LEN] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
  ESP_LOGI(TAG, "Inserting MAC1");
  mac_table_insert(&table, mac1);

  // Wait 2 seconds, then update the same MAC
  vTaskDelay(pdMS_TO_TICKS(2000));
  ESP_LOGI(TAG, "Updating MAC1");
  mac_table_insert(&table, mac1);

  // Wait 3 seconds, should not expire yet (5 seconds from update)
  vTaskDelay(pdMS_TO_TICKS(3000));
  ESP_LOGI(TAG, "Waiting 3 seconds");

  // Test 2: Insert another MAC
  uint8_t mac2[MAC_ADDR_LEN] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
  uint8_t mac3[MAC_ADDR_LEN] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
  ESP_LOGI(TAG, "Inserting MAC2");
  mac_table_insert(&table, mac2);

  // Test 3: Delete MAC1
  vTaskDelay(pdMS_TO_TICKS(1000));
  ESP_LOGI(TAG, "Deleting MAC1");
  mac_table_delete(&table, mac1);

  // Wait 5 seconds to observe MAC2 expiration
  vTaskDelay(pdMS_TO_TICKS(5000));
  ESP_LOGI(TAG, "Test completed");
  mac_table_insert(&table, mac3);
  vTaskDelay(pdMS_TO_TICKS(5000));

  mac_table_insert(&table, mac3);
  vTaskDelay(pdMS_TO_TICKS(5000));
  mac_table_insert(&table, mac1);
  uint8_t mac4[MAC_ADDR_LEN] = {0x00, 0x22, 0x22, 0x33, 0x44, 0x55};
  uint8_t mac5[MAC_ADDR_LEN] = {0x00, 0x22, 0x11, 0x33, 0x44, 0x55};
  uint8_t mac6[MAC_ADDR_LEN] = {0x00, 0x22, 0x11, 0x33, 0x11, 0x55};
  uint8_t mac7[MAC_ADDR_LEN] = {0x00, 0x22, 0x11, 0x33, 0x11, 0x44};
  uint8_t mac8[MAC_ADDR_LEN] = {0x11, 0x22, 0x11, 0x33, 0x11, 0x55};
  uint8_t mac9[MAC_ADDR_LEN] = {0x11, 0xFF, 0x11, 0x33, 0x11, 0x55};
  uint8_t mac10[MAC_ADDR_LEN] = {0x11, 0xFF, 0x11, 0x33, 0x11, 0x11};
  uint8_t mac11[MAC_ADDR_LEN] = {0x11, 0xFF, 0xFF, 0x33, 0x11, 0x11};
  uint8_t mac12[MAC_ADDR_LEN] = {0x11, 0xFF, 0xFF, 0x33, 0x11, 0xFF};
  mac_table_insert(&table, mac4);
  mac_table_insert(&table, mac5);
  mac_table_insert(&table, mac6);
  mac_table_insert(&table, mac6);
  mac_table_insert(&table, mac7);
  mac_table_insert(&table, mac4);
  mac_table_insert(&table, mac8);
  mac_table_insert(&table, mac9);
  mac_table_insert(&table, mac8);

  mac_table_insert(&table, mac10);
  vTaskDelay(pdMS_TO_TICKS(1000));
  mac_table_insert(&table, mac11);
  vTaskDelay(pdMS_TO_TICKS(1000));
  mac_table_insert(&table, mac12);
  vTaskDelay(pdMS_TO_TICKS(5000));
  mac_table_delete(&table, mac10);
  vTaskDelay(pdMS_TO_TICKS(5000));
  mac_table_delete(&table, mac12);

  while (true) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
