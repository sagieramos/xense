#include "xense_utils.h"

/**
 * @brief Control a FreeRTOS task.
 *
 * This function can be used to resume, suspend, delete, or restart a FreeRTOS
 * task.
 *
 * @param task The task handle to control.
 * @param action The action to perform on the task (resume, suspend, delete,
 * restart).
 * @param taskFunction The function to run in the task (only used for restart).
 * @param taskName The name of the task (only used for restart).
 * @param stackDepth The stack depth for the task (only used for restart).
 * @param parameters The parameters to pass to the task function (only used for
 * restart).
 * @param priority The priority of the task (only used for restart).
 */

void control_task(TaskHandle_t &task, TaskControlAction action,
                  TaskFunction_t taskFunction, const char *taskName,
                  uint32_t stackDepth, void *parameters, UBaseType_t priority) {
  if (task == NULL && action != TASK_RESTART) {
    LOG_LN("Task is NULL");
    return;
  }

  switch (action) {
  case TASK_RESUME:
    if (eTaskGetState(task) == eSuspended) {
      vTaskResume(task);
      LOG_F("Resumed task: %s\n", pcTaskGetTaskName(task));
    } else {
      LOG_F("Task already running: %s\n", pcTaskGetTaskName(task));
    }
    break;

  case TASK_SUSPEND:
    if (eTaskGetState(task) != eSuspended) {
      vTaskSuspend(task);
      LOG_F("Suspended task: %s\n", pcTaskGetTaskName(task));
    } else {
      LOG_F("Task already suspended: %s\n", pcTaskGetTaskName(task));
    }
    break;

  case TASK_DELETE:
    vTaskDelete(task);
    LOG_F("Deleted task\n");
    task = NULL;
    break;

  case TASK_RESTART:
    if (task != NULL) {
      vTaskDelete(task);
      LOG_F("Deleted task for restart\n");
      task = NULL;
    }
    if (taskFunction != nullptr && taskName != nullptr) {
      xTaskCreatePinnedToCore(taskFunction, taskName, stackDepth, parameters,
                              priority, &task, 1);
      LOG_F("Restarted task: %s\n", taskName);
    } else {
      LOG_LN("Cannot restart task: taskFunction or taskName is NULL");
    }
    break;
  }
}

enum TimeUnit {
  TIME_UNIT_MICROSECONDS,
  TIME_UNIT_MILLISECONDS,
  TIME_UNIT_SECONDS,
  TIME_UNIT_MINUTES,
  TIME_UNIT_HOURS,
  TIME_UNIT_DAYS
};

/**
 * @brief Get the current time in milliseconds.
 *
 * This function returns the current time in milliseconds since the system
 * started.
 *
 * @return The current time in milliseconds.
 */

unsigned long get_current_ms() {
  return xTaskGetTickCount() * portTICK_PERIOD_MS;
}
