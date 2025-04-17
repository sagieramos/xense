#ifndef XENSE_UTILS_H
#define XENSE_UTILS_H

#include <Arduino.h>

#ifdef LOG_ENABLED
#define LOG_BEGIN(baud) Serial.begin(baud)
#define LOG_(...) Serial.print(__VA_ARGS__)
#define LOG_LN(...) Serial.println(__VA_ARGS__)
#define LOG_F(...) Serial.printf(__VA_ARGS__)
#else
#define LOG_BEGIN(baud) ((void)0)
#define LOG_(...) ((void)0)
#define LOG_LN(...) ((void)0)
#define LOG_F(...) ((void)0)
#endif

enum TaskControlAction { TASK_RESUME, TASK_SUSPEND, TASK_DELETE, TASK_RESTART };

void control_task(TaskHandle_t &task, TaskControlAction action,
                  TaskFunction_t taskFunction = nullptr,
                  const char *taskName = nullptr, uint32_t stackDepth = 2048,
                  void *parameters = nullptr, UBaseType_t priority = 1);
unsigned long get_current_ms();

#endif // UTILS_H