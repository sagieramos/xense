#ifndef LOG_H
#define LOG_H

#include "esp_log.h"

#ifdef LOG_ENABLED
#define LOG_XENSE(tag, ...) ESP_LOGI(tag, __VA_ARGS__)
#else
#define LOG_XENSE(tag, ...) ((void)0)
#endif

#endif // LOG_H