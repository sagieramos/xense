#include "xense_utils.h"
#include "esp_mac.h"

#define TASK "TASK"
#define LED_QUEUE_LENGTH 5
static QueueHandle_t led_cmd_queue;

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
    LOG_XENSE(TASK, "NULL");
    return;
  }

  switch (action) {
  case TASK_RESUME:
    if (eTaskGetState(task) == eSuspended) {
      vTaskResume(task);

      LOG_XENSE(TASK, "%s Resume\n", pcTaskGetName(task));
    } else {
      LOG_XENSE(TASK, "%s already running\n", pcTaskGetName(task));
    }
    break;

  case TASK_SUSPEND:
    if (eTaskGetState(task) != eSuspended) {
      vTaskSuspend(task);
      LOG_XENSE(TASK, "%s suspended\n", pcTaskGetName(task));
    } else {
      LOG_XENSE(TASK, "%s already suspended\n", pcTaskGetName(task));
    }
    break;

  case TASK_DELETE:
    vTaskDelete(task);
    LOG_XENSE("Deleted task\n");
    task = NULL;
    break;

  case TASK_RESTART:
    if (task != NULL) {
      vTaskDelete(task);
      LOG_XENSE("Deleted task for restart\n");
      task = NULL;
    }
    if (taskFunction != nullptr && taskName != nullptr) {
      xTaskCreatePinnedToCore(taskFunction, taskName, stackDepth, parameters,
                              priority, &task, 1);
      LOG_XENSE(TASK, "%s Restarted\n", taskName);
    } else {
      LOG_XENSE(TASK, "Cannot restart: taskFunction or taskName is NULL");
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

static void led_task(void *arg) {
  gpio_reset_pin(LED_INDICATOR);
  gpio_set_direction(LED_INDICATOR, GPIO_MODE_OUTPUT);

  led_msg_t current_cmd = {.command = LED_CMD_BLINK_CUSTOM,
                           .on_duration_ms = 500,
                           .off_duration_ms = 500};

  while (true) {
    led_msg_t new_cmd;
    if (xQueueReceive(led_cmd_queue, &new_cmd, 0)) {
      current_cmd = new_cmd;
    }

    switch (current_cmd.command) {
    case LED_CMD_BLINK_CUSTOM:
      gpio_set_level(LED_INDICATOR, 1);
      vTaskDelay(current_cmd.on_duration_ms / portTICK_PERIOD_MS);
      gpio_set_level(LED_INDICATOR, 0);
      vTaskDelay(current_cmd.off_duration_ms / portTICK_PERIOD_MS);
      break;
    case LED_CMD_SOLID_ON:
      gpio_set_level(LED_INDICATOR, 1);
      vTaskDelay(100 / portTICK_PERIOD_MS);
      break;
    case LED_CMD_SOLID_OFF:
      gpio_set_level(LED_INDICATOR, 0);
      vTaskDelay(100 / portTICK_PERIOD_MS);
      break;
    }
  }
}

void led_indicator_control(led_cmd_t command, uint32_t on_duration_ms,
                           uint32_t off_duration_ms) {
  led_msg_t msg = {.command = command,
                   .on_duration_ms = on_duration_ms,
                   .off_duration_ms = off_duration_ms};

  xQueueSend(led_cmd_queue, &msg, 0);
}

void init_led_cmd() {
  gpio_reset_pin(LED_INDICATOR);
  gpio_set_direction(LED_INDICATOR, GPIO_MODE_OUTPUT);
  led_cmd_queue = xQueueCreate(LED_QUEUE_LENGTH, sizeof(led_msg_t));
  xTaskCreate(led_task, "led_task", 2048, NULL, 5, NULL);
}