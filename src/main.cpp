#include "esp_event.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_netif.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "xense_utils.h"
#include <stdio.h>
#include <stdlib.h>

#define MAX_CLIENTS 10

static const char *TAG = "WebSocketServer";
static httpd_handle_t server = NULL;

static int ws_clients[MAX_CLIENTS];
static size_t ws_client_count = 0;
static SemaphoreHandle_t clients_mutex;

// === DASHBOARD HTML ===
static const char *dashboard_html =
    "<!DOCTYPE html>"
    "<html><head><title>ESP Dashboard</title></head><body>"
    "<h2>ESP32 WebSocket Random Numbers</h2>"
    "<pre id='output'></pre>"
    "<input id='input' type='text' placeholder='Send message...'/>"
    "<button onclick='send()'>Send</button>"
    "<script>"
    "const ws = new WebSocket('ws://' + location.host + '/ws');"
    "ws.onmessage = (event) => { document.getElementById('output').textContent "
    "+= event.data + '\\n'; };"
    "function send() { ws.send(document.getElementById('input').value); }"
    "</script>"
    "</body></html>";

// === DASHBOARD HANDLER ===
esp_err_t dashboard_get_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, dashboard_html, HTTPD_RESP_USE_STRLEN);
}

httpd_uri_t dashboard = {.uri = "/",
                         .method = HTTP_GET,
                         .handler = dashboard_get_handler,
                         .user_ctx = NULL,
                         .is_websocket = false,
                         .handle_ws_control_frames = false,
                         .supported_subprotocol = NULL};

// === WEBSOCKET HANDLER ===
static esp_err_t ws_handler(httpd_req_t *req) {
  if (req->method == HTTP_GET) {
    return ESP_OK;
  }

  httpd_ws_frame_t frame = {.final = true,
                            .fragmented = false,
                            .type = HTTPD_WS_TYPE_TEXT,
                            .payload = NULL,
                            .len = 0};

  esp_err_t err = httpd_ws_recv_frame(req, &frame, 0);
  if (err == ESP_OK && frame.len > 0) {
    frame.payload = (uint8_t *)malloc(frame.len + 1);
    if (!frame.payload) {
      ESP_LOGE(TAG, "Memory allocation failed");
      return ESP_ERR_NO_MEM;
    }
    err = httpd_ws_recv_frame(req, &frame, frame.len);
    if (err == ESP_OK) {
      frame.payload[frame.len] = 0;
      ESP_LOGI(TAG, "Received from client [%d]: %s", httpd_req_to_sockfd(req),
               (char *)frame.payload);
      httpd_ws_send_frame(req, &frame);
    }
    free(frame.payload);
  }

  int sock_fd = httpd_req_to_sockfd(req);
  xSemaphoreTake(clients_mutex, portMAX_DELAY);
  bool exists = false;
  for (size_t i = 0; i < ws_client_count; ++i) {
    if (ws_clients[i] == sock_fd) {
      exists = true;
      break;
    }
  }
  if (!exists && ws_client_count < MAX_CLIENTS) {
    ws_clients[ws_client_count++] = sock_fd;
    ESP_LOGI(TAG, "New WebSocket client added: %d", sock_fd);
  }
  xSemaphoreGive(clients_mutex);

  return ESP_OK;
}

// === BROADCAST FUNCTION ===
static void broadcast_ws_message(void *arg) {
  char *msg = (char *)arg;
  httpd_ws_frame_t frame = {.final = true,
                            .fragmented = false,
                            .type = HTTPD_WS_TYPE_TEXT,
                            .payload = (uint8_t *)msg,
                            .len = strlen(msg)};

  xSemaphoreTake(clients_mutex, portMAX_DELAY);
  for (size_t i = 0; i < ws_client_count; ++i) {
    int sock = ws_clients[i];
    if (httpd_ws_get_fd_info(server, sock) == HTTPD_WS_CLIENT_WEBSOCKET) {
      esp_err_t ret = httpd_ws_send_frame_async(server, sock, &frame);
      if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to send to client [%d]: %s", sock,
                 esp_err_to_name(ret));
      }
    } else {
      ESP_LOGI(TAG, "Removing disconnected client [%d]", sock);
      for (size_t j = i; j < ws_client_count - 1; ++j) {
        ws_clients[j] = ws_clients[j + 1];
      }
      ws_client_count--;
      i--;
    }
  }
  xSemaphoreGive(clients_mutex);
  free(msg);
}

// === PERIODIC BROADCAST TASK ===
static void websocket_broadcast_task(void *arg) {
  while (1) {
    int num = esp_random() % 1000;
    char *msg = (char *)malloc(32);
    sprintf(msg, "{\"random\": %d}", num);
    httpd_queue_work(server, broadcast_ws_message, msg);
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

// === SERVER START ===
static httpd_handle_t start_webserver(void) {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.stack_size = 8192;

  ESP_LOGI(TAG, "Starting HTTP server on port %d", config.server_port);
  if (httpd_start(&server, &config) == ESP_OK) {
    httpd_uri_t ws_uri = {.uri = "/ws",
                          .method = HTTP_GET,
                          .handler = ws_handler,
                          .user_ctx = NULL,
                          .is_websocket = true,
                          .handle_ws_control_frames = true,
                          .supported_subprotocol = NULL};
    httpd_register_uri_handler(server, &dashboard);
    httpd_register_uri_handler(server, &ws_uri);
  }

  return server;
}

// === APP MAIN ===
extern "C" void app_main(void);
uint8_t mac[6];

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

  /*   ESP_ERROR_CHECK(esp_read_mac(mac, ESP_MAC_WIFI_STA));
    ESP_LOGI(TAG, "MAC Address: %02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1],
             mac[2], mac[3], mac[4], mac[5]); */

  // clients_mutex = xSemaphoreCreateMutex();

  // wifi_init_sta();
  // wifi_init_ap();
  wifi_init_ap_sta();

  // start_webserver();

  // xTaskCreate(websocket_broadcast_task, "ws_broadcast", 4096, NULL, 5, NULL);

  while (true) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
