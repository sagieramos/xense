#include "server.h"

httpd_handle_t server = NULL;

const char *WEBSERVER_TAG = "WEBSERVER_SERVER";

void *setup_server() {
  struct {
    const char *uri;
    httpd_method_t method;
    esp_err_t (*handler)(httpd_req_t *r);
    bool is_websocket;
  } routes[] = {{"/", HTTP_GET, get_req_handler, false},
                {"/favicon.ico", HTTP_GET, favicon_get_handler, false},
                {"/style.css", HTTP_GET, style_get_handler, false},
                {"/app.js", HTTP_GET, appjs_get_handler, false},
                {"/ws", HTTP_GET, handle_ws_req, true}};

  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  if (httpd_start(&server, &config) == ESP_OK) {
    for (size_t i = 0; i < sizeof(routes) / sizeof(routes[0]); ++i) {
      static httpd_uri_t uri_handler;

      uri_handler.uri = routes[i].uri;
      uri_handler.method = routes[i].method;
      uri_handler.handler = routes[i].handler;
      uri_handler.user_ctx = NULL;
      uri_handler.is_websocket = routes[i].is_websocket;
      uri_handler.handle_ws_control_frames = NULL;
      uri_handler.supported_subprotocol = NULL;

      httpd_register_uri_handler(server, &uri_handler);
    }
  }

  return server;
}
