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
#include "log.h"
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

  while (true) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
