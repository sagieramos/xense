#include "xense_utils.h"
#include "esp_log.h"
#include "esp_mac.h"

#define TASK "TASK"
#define LED_QUEUE_LENGTH 5
static QueueHandle_t led_cmd_queue;

/**
 * @brief Control a FreeRTOS task.
 */
void control_task(TaskHandle_t *task, TaskControlAction action,
                  TaskFunction_t taskFunction, const char *taskName,
                  uint32_t stackDepth, void *parameters, UBaseType_t priority) {
  if (task == NULL && action != TASK_RESTART) {
    ESP_LOGE(TASK, "Task handle is NULL");
    return;
  }

  switch (action) {
  case TASK_RESUME:
    if (eTaskGetState(*task) == eSuspended) {
      vTaskResume(*task);
      ESP_LOGI(TASK, "%s Resume", pcTaskGetName(*task));
    } else {
      ESP_LOGW(TASK, "%s already running", pcTaskGetName(*task));
    }
    break;

  case TASK_SUSPEND:
    if (eTaskGetState(*task) != eSuspended) {
      vTaskSuspend(*task);
      ESP_LOGI(TASK, "%s suspended", pcTaskGetName(*task));
    } else {
      ESP_LOGW(TASK, "%s already suspended", pcTaskGetName(*task));
    }
    break;

  case TASK_DELETE:
    vTaskDelete(*task);
    ESP_LOGI(TASK, "Deleted task");
    *task = NULL;
    break;

  case TASK_RESTART:
    if (*task != NULL) {
      vTaskDelete(*task);
      ESP_LOGI(TASK, "Deleted task for restart");
      *task = NULL;
    }
    if (taskFunction != nullptr && taskName != nullptr) {
      xTaskCreatePinnedToCore(taskFunction, taskName, stackDepth, parameters,
                              priority, task, 1);
      ESP_LOGI(TASK, "%s Restarted", taskName);
    } else {
      ESP_LOGE(TASK, "Cannot restart: taskFunction or taskName is NULL");
    }
    break;
  }
}

/**
 * @brief Get the current time in milliseconds.
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

  led_msg_t previous_cmd = current_cmd;
  bool blink_once_triggered = false;

  while (true) {
    led_msg_t new_cmd;
    if (xQueueReceive(led_cmd_queue, &new_cmd,
                      blink_once_triggered ? 0 : pdMS_TO_TICKS(50))) {
      if (new_cmd.command != LED_CMD_BLINK_ONCE) {
        previous_cmd = new_cmd;
      }
      current_cmd = new_cmd;
    }

    switch (current_cmd.command) {
    case LED_CMD_BLINK_ONCE: {
      blink_once_triggered = true;
      int initial_state = gpio_get_level(LED_INDICATOR);

      gpio_set_level(LED_INDICATOR, !initial_state);
      vTaskDelay(pdMS_TO_TICKS(current_cmd.on_duration_ms));

      gpio_set_level(LED_INDICATOR, initial_state);
      vTaskDelay(pdMS_TO_TICKS(current_cmd.off_duration_ms));

      // Revert to previous mode
      current_cmd = previous_cmd;
      blink_once_triggered = false;
      break;
    }
    case LED_CMD_BLINK_CUSTOM:
      gpio_set_level(LED_INDICATOR, 1);
      vTaskDelay(pdMS_TO_TICKS(current_cmd.on_duration_ms));
      gpio_set_level(LED_INDICATOR, 0);
      vTaskDelay(pdMS_TO_TICKS(current_cmd.off_duration_ms));
      break;

    case LED_CMD_SOLID_ON:
      gpio_set_level(LED_INDICATOR, 1);
      vTaskDelay(pdMS_TO_TICKS(200));
      break;

    case LED_CMD_SOLID_OFF:
      gpio_set_level(LED_INDICATOR, 0);
      vTaskDelay(pdMS_TO_TICKS(200));
      break;

    default:
      ESP_LOGW("LED", "Unknown command received: %d", current_cmd.command);
      gpio_set_level(LED_INDICATOR, 0);
      vTaskDelay(pdMS_TO_TICKS(200));
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
