#ifndef XENSE_UTILS_H
#define XENSE_UTILS_H

#include <string.h>
#include "esp_https_server.h"
#include "esp_log.h"
#include "esp_tls.h"
#include "esp_netif.h"
#include "nvs_flash.h"

#ifdef LOG_ENABLED
#define LOG_(tag, ...) esp_log_write(ESP_LOG_INFO, tag, __VA_ARGS__)
#define LOG_LN(tag, ...) esp_log_write(ESP_LOG_INFO, tag, __VA_ARGS__)
#define LOG_F(tag, ...) ESP_LOGI(tag, __VA_ARGS__)
#else
#define LOG_BEGIN(baud) ((void)0)
#define LOG_(tag, ...) ((void)0)
#define LOG_LN(tag, ...) ((void)0)
#define LOG_F(tag, ...) ((void)0)
#endif

enum TaskControlAction { TASK_RESUME, TASK_SUSPEND, TASK_DELETE, TASK_RESTART };

void control_task(TaskHandle_t &task, TaskControlAction action,
                  TaskFunction_t taskFunction = nullptr,
                  const char *taskName = nullptr, uint32_t stackDepth = 2048,
                  void *parameters = nullptr, UBaseType_t priority = 1);
unsigned long get_current_ms();

#endif // UTILS_H