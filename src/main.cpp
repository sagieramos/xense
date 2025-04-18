#include "xense.h"
#include "xense_utils.h"

#include <string.h>
#include "esp_log.h"
#include "esp_https_server.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "esp_mac.h"

static const char *TAG = "HTTPS_SERVER";

// Embed certs
extern const uint8_t server_cert_pem_start[] asm("_binary_server_cert_pem_start");
extern const uint8_t server_cert_pem_end[]   asm("_binary_server_cert_pem_end");
extern const uint8_t server_key_pem_start[]  asm("_binary_server_key_pem_start");
extern const uint8_t server_key_pem_end[]    asm("_binary_server_key_pem_end");

// Simple HTTPS GET handler
esp_err_t hello_get_handler(httpd_req_t *req) {
    const char* resp_str = "<html><body><h1>Hello from ESP32 Secure Server!</h1></body></html>";
    httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

httpd_uri_t hello_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = hello_get_handler,
    .user_ctx  = NULL
};

// Initialize HTTPS server
void start_https_server(void) {
    httpd_ssl_config_t conf = HTTPD_SSL_CONFIG_DEFAULT();
    conf.servercert = server_cert_pem_start;
    conf.servercert_len = server_cert_pem_end - server_cert_pem_start;
    conf.prvtkey_pem = server_key_pem_start;
    conf.prvtkey_len = server_key_pem_end - server_key_pem_start;

    httpd_handle_t server = NULL;
    if (httpd_ssl_start(&server, &conf) == ESP_OK) {
        httpd_register_uri_handler(server, &hello_uri);
        ESP_LOGI(TAG, "HTTPS server started");
    } else {
        ESP_LOGE(TAG, "Failed to start HTTPS server");
    }
}

void app_main(void) {
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_ap();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = "ESP32_AP",
            .password = "12345678",
            .ssid_len = strlen("ESP32_AP"),
            .channel = 1,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
            .max_connection = 2
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi AP started. Connect to https://192.168.4.1/");

    start_https_server();
}
