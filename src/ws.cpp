#include "custom_wifi.h"
#include "log.h"
#include "server.h"
#include "xense_utils.h"

const char *TAG = "WebSocket Server";
static TickType_t last_scan_tick = 0;
static portMUX_TYPE scan_mux = portMUX_INITIALIZER_UNLOCKED;

httpd_handle_t ws_server_handle = NULL;
int ws_socket_fd = -1;

void ws_async_send(void *arg) {
  async_resp_arg_t *resp_arg = (async_resp_arg_t *)arg;
  httpd_handle_t hd = resp_arg->hd;
  httpd_ws_frame_t ws_pkt;

  memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
  ws_pkt.payload = resp_arg->payload;
  ws_pkt.len = resp_arg->payload_len;
  ws_pkt.type = resp_arg->is_binary ? HTTPD_WS_TYPE_BINARY : HTTPD_WS_TYPE_TEXT;

  if (resp_arg->mode == WS_SEND_MODE_BROADCAST) {
    // Broadcast to all connected clients
    static size_t max_clients = CONFIG_LWIP_MAX_LISTENING_TCP;
    size_t fds = max_clients;
    int client_fds[max_clients];

    if (httpd_get_client_list(hd, &fds, client_fds) == ESP_OK) {
      for (int i = 0; i < fds; i++) {
        if (httpd_ws_get_fd_info(hd, client_fds[i]) ==
            HTTPD_WS_CLIENT_WEBSOCKET) {
          httpd_ws_send_frame_async(hd, client_fds[i], &ws_pkt);
        }
      }
    }
  } else if (resp_arg->mode == WS_SEND_MODE_UNICAST) {
    // Unicast to a single client
    httpd_ws_send_frame_async(hd, resp_arg->fd, &ws_pkt);
  }

  free(resp_arg);
}

esp_err_t ws_trigger_async_send(httpd_handle_t server, httpd_req_t *req,
                                const uint8_t *data, size_t len,
                                ws_send_mode_t mode, bool is_binary) {
  async_resp_arg_t *resp_arg =
      (async_resp_arg_t *)malloc(sizeof(async_resp_arg_t));
  if (!resp_arg) {
    return ESP_ERR_NO_MEM;
  }

  resp_arg->hd = server;
  resp_arg->mode = mode;
  resp_arg->is_binary = is_binary;

  // Copy payload (limit to internal buffer size)
  resp_arg->payload_len =
      len > sizeof(resp_arg->payload) ? sizeof(resp_arg->payload) : len;
  memcpy(resp_arg->payload, data, resp_arg->payload_len);

  if (mode == WS_SEND_MODE_UNICAST) {
    if (req != NULL) {
      resp_arg->fd = httpd_req_to_sockfd(req);
    } else if (ws_socket_fd != -1) {
      resp_arg->fd = ws_socket_fd;
    } else {
      free(resp_arg);
      LOG_XENSE("WebSocket Server",
                "Invalid argument: req is NULL and ws_socket_fd is not set");
      return ESP_ERR_INVALID_ARG;
    }
  } else {
    resp_arg->fd = -1; // For broadcast mode
  }

  return httpd_queue_work(server, ws_async_send, resp_arg);
}

esp_err_t handle_ws_req(httpd_req_t *req) {
  if (req->method == HTTP_GET) {
    LOG_XENSE(TAG, "Handshake done, a new connection was opened");
    return ESP_OK;
  }

  led_indicator_control(LED_CMD_BLINK_ONCE, 100, 100);

  httpd_ws_frame_t ws_pkt;
  uint8_t *buf = NULL;
  esp_err_t ret;

  memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));

  ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
  if (ret != ESP_OK) {
    LOG_XENSE(TAG,
              "httpd_ws_recv_frame failed to get frame length with error %d",
              ret);
    return ret;
  }

  LOG_XENSE(TAG, "Received frame of type %d and length %d", ws_pkt.type,
            ws_pkt.len);

  if (ws_pkt.len > 0) {
    buf = (uint8_t *)calloc(1, ws_pkt.len + 1);
    if (buf == NULL) {
      LOG_XENSE(TAG, "Failed to allocate memory for frame payload");
      return ESP_ERR_NO_MEM;
    }

    ws_pkt.payload = buf;
    ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
    if (ret != ESP_OK) {
      LOG_XENSE(TAG,
                "httpd_ws_recv_frame failed to receive payload with error %d",
                ret);
      free(buf);
      return ret;
    }
  }

  if (ws_pkt.type == HTTPD_WS_TYPE_TEXT) {
    LOG_XENSE(TAG, "Received TEXT message: %s",
              ws_pkt.payload ? (char *)ws_pkt.payload : "(null)");

    if (ws_pkt.payload && strcmp((char *)ws_pkt.payload, "scan") == 0) {
      ws_server_handle = req->handle;
      ws_socket_fd = httpd_req_to_sockfd(req);

      // Use a mutex to protect the scan cooldown
      TickType_t now = xTaskGetTickCount();
      taskENTER_CRITICAL(&scan_mux);
      if (now - last_scan_tick < pdMS_TO_TICKS(2000)) {
        taskEXIT_CRITICAL(&scan_mux);
        LOG_XENSE(TAG, "Scan request ignored due to cooldown");
        free(buf);
        return ESP_OK;
      }
      last_scan_tick = now;
      taskEXIT_CRITICAL(&scan_mux);

      LOG_XENSE(TAG, "Received scan request");
      wifi_scan(); // Start scan
      free(buf);
      return ESP_OK;
    }

  } else if (ws_pkt.type == HTTPD_WS_TYPE_BINARY) {
    LOG_XENSE(TAG, "Received BINARY message of %d bytes", ws_pkt.len);
    // TODO: Handle binary payload here (e.g., parse Protocol Buffers, etc.)
  } else {
    LOG_XENSE(TAG, "Unhandled WebSocket frame type: %d", ws_pkt.type);
  }

  if (buf) {
    free(buf);
  }
  return ESP_OK;
}