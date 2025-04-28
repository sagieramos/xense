#ifndef CUSTOM_SERVER_H
#define CUSTOM_SERVER_H
#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include <cstring>

typedef enum { WS_SEND_MODE_BROADCAST, WS_SEND_MODE_UNICAST } ws_send_mode_t;

typedef struct {
  httpd_handle_t hd;    // HTTP server handle
  int fd;               // Socket file descriptor
  ws_send_mode_t mode;  // Send mode (broadcast or unicast)
  size_t payload_len;   // Length of the payload
  uint8_t payload[256]; // Buffer big enough for your payload
  bool is_binary;       // true if payload is binary (protobuf)
} async_resp_arg_t;

esp_err_t ws_trigger_async_send(httpd_handle_t server, httpd_req_t *req,
                                const uint8_t *data, size_t len,
                                ws_send_mode_t mode, bool is_binary);

extern httpd_handle_t server;
extern const char *WEBSERVER_TAG;

extern httpd_handle_t ws_server_handle;
extern int ws_socket_fd;

esp_err_t get_req_handler(httpd_req_t *req);
esp_err_t handle_ws_req(httpd_req_t *req);
esp_err_t favicon_get_handler(httpd_req_t *req);
esp_err_t style_get_handler(httpd_req_t *req);
esp_err_t appjs_get_handler(httpd_req_t *req);

void init_web_page_buffers();
httpd_handle_t setup_server();

#endif // CUSTOM_SERVER_H
