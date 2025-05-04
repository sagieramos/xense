#include "server.h"

char index_html[4096];
char app_js[4096];
char style_css[4096];
char favicon_ico[4096];

#define INDEX_HTML_PATH "/spiffs/index.html"
#define APP_JS_PATH "/spiffs/app.js"
#define STYLE_CSS_PATH "/spiffs/style.css"
#define FAVICON_ICO_PATH "/spiffs/favicon.ico"

esp_err_t get_req_handler(httpd_req_t *req) {
  int response;

  response = httpd_resp_send(req, index_html, HTTPD_RESP_USE_STRLEN);
  return response;
}

esp_err_t favicon_get_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "image/x-icon");
  httpd_resp_send(req, favicon_ico, strlen(favicon_ico));
  return ESP_OK;
}

esp_err_t style_get_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "text/css");
  httpd_resp_send(req, style_css, strlen(style_css));
  return ESP_OK;
}

esp_err_t appjs_get_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "application/javascript");
  httpd_resp_send(req, app_js, strlen(app_js));
  return ESP_OK;
}

void init_web_page_buffers() {
  esp_vfs_spiffs_conf_t conf = {.base_path = "/spiffs",
                                .partition_label = NULL,
                                .max_files = 10,
                                .format_if_mount_failed = true};

  ESP_ERROR_CHECK(esp_vfs_spiffs_register(&conf));

  struct {
    const char *path;
    uint8_t *buffer;
    size_t buffer_size;
    const char *file_name;
  } files_to_load[] = {
      {INDEX_HTML_PATH, (uint8_t *)index_html, sizeof(index_html),
       "index.html"},
      {APP_JS_PATH, (uint8_t *)app_js, sizeof(app_js), "app.js"},
      {STYLE_CSS_PATH, (uint8_t *)style_css, sizeof(style_css), "style.css"},
      {FAVICON_ICO_PATH, (uint8_t *)favicon_ico, sizeof(favicon_ico),
       "favicon.ico"}};

  for (int i = 0; i < sizeof(files_to_load) / sizeof(files_to_load[0]); i++) {
    struct stat st;
    if (stat(files_to_load[i].path, &st)) {
      ESP_LOGW(WEBSERVER_TAG, "%s not found", files_to_load[i].file_name);
      continue;
    }

    if (st.st_size >= files_to_load[i].buffer_size) {
      ESP_LOGW(WEBSERVER_TAG, "%s too large for buffer",
               files_to_load[i].file_name);
      continue;
    }

    FILE *fp = fopen(files_to_load[i].path, "r");
    if (!fp) {
      ESP_LOGE(WEBSERVER_TAG, "Failed to open %s", files_to_load[i].file_name);
      continue;
    }

    memset(files_to_load[i].buffer, 0, files_to_load[i].buffer_size);
    if (fread(files_to_load[i].buffer, st.st_size, 1, fp) == 0) {
      ESP_LOGE(WEBSERVER_TAG, "Failed to read %s", files_to_load[i].file_name);
    }
    fclose(fp);
  }
}
