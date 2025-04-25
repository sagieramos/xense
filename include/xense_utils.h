#ifndef XENSE_UTILS_H
#define XENSE_UTILS_H

#include "driver/gpio.h"
#include "esp_https_server.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_netif.h"
#include "esp_tls.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include <string.h>

#ifdef LOG_ENABLED
#define LOG_XENSE(tag, ...) ESP_LOGI(tag, __VA_ARGS__)
#else
#define LOG_XENSE(tag, ...) ((void)0)
#endif

const gpio_num_t LED_INDICATOR = GPIO_NUM_2;

typedef enum {
  TASK_RESUME,
  TASK_SUSPEND,
  TASK_DELETE,
  TASK_RESTART
} TaskControlAction;

typedef enum {
  LED_CMD_BLINK_CUSTOM,
  LED_CMD_SOLID_ON,
  LED_CMD_SOLID_OFF
} led_cmd_t;

typedef struct {
  led_cmd_t command;
  uint32_t on_duration_ms;  // Used in BLINK_CUSTOM
  uint32_t off_duration_ms; // Used in BLINK_CUSTOM
} led_msg_t;

void control_task(TaskHandle_t &task, TaskControlAction action,
                  TaskFunction_t taskFunction, const char *taskName,
                  uint32_t stackDepth, void *parameters, UBaseType_t priority);

unsigned long get_current_ms();

void wifi_init_sta();
void wifi_init_ap();
void wifi_init_ap_sta();
void wifi_scan();
void init_led_cmd();

void led_indicator_control(led_cmd_t command, uint32_t on_duration_ms,
                           uint32_t off_duration_ms);

#endif // XENSE_UTILS_H
