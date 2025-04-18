#include "xense_utils.h"

#define TASK "TASK"

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
    LOG_LN(TASK, "NULL");
    return;
  }

  switch (action) {
  case TASK_RESUME:
    if (eTaskGetState(task) == eSuspended) {
      vTaskResume(task);
      
      LOG_F(TASK, "%s Resume\n", pcTaskGetName(task));
    } else {
      LOG_F(TASK, "%s already running\n", pcTaskGetName(task));
    }
    break;

  case TASK_SUSPEND:
    if (eTaskGetState(task) != eSuspended) {
      vTaskSuspend(task);
      LOG_F(TASK, "%s suspended\n", pcTaskGetName(task));
    } else {
      LOG_F(TASK, "%s already suspended\n", pcTaskGetName(task));
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
      LOG_F(TASK, "%s Restarted\n", taskName);
    } else {
      LOG_LN(TASK, "Cannot restart: taskFunction or taskName is NULL");
    }
    break;
  }
}

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
