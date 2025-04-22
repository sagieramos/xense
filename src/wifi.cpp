#include "config.h"
#include "esp_wifi.h"
#include "xense_utils.h"

#define TAG "WiFiModule"

#define WIFI_CONNECTED_BIT BIT0
static EventGroupHandle_t wifi_event_group;

// === LED BLINK ===
static wifi_sta_config_t sta_config = {
    .ssid = WIFI_SSID,
    .password = WIFI_PASS,
    .scan_method = WIFI_ALL_CHANNEL_SCAN,
    .bssid_set = false,
    .bssid = {0},
    .channel = 0,
    .listen_interval = 3,
    .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
    .threshold = {.rssi = -127,
                  .authmode = WIFI_AUTH_OPEN,
                  .rssi_5g_adjustment = 0},
    .pmf_cfg = {.capable = true, .required = false},
    .rm_enabled = false,
    .btm_enabled = false,
    .mbo_enabled = false,
    .ft_enabled = false,
    .owe_enabled = false,
    .transition_disable = false,
    .reserved = 0,
    .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
    .sae_pk_mode = WPA3_SAE_PK_MODE_AUTOMATIC,
    .failure_retry_cnt = 3,
    .he_dcm_set = false,
    .he_dcm_max_constellation_tx = 0,
    .he_dcm_max_constellation_rx = 0,
    .he_mcs9_enabled = false,
    .he_su_beamformee_disabled = false,
    .he_trig_su_bmforming_feedback_disabled = false,
    .he_trig_mu_bmforming_partial_feedback_disabled = false,
    .he_trig_cqi_feedback_disabled = false,
    .he_reserved = 0,
    .sae_h2e_identifier = ""};

// === WIFI EVENT HANDLER ===
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data) {
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    LOG_F(TAG, "Wi-Fi started. Connecting...");
    esp_wifi_connect();

    // Fast blink while trying to connect
    // led_indicator_control(LED_CMD_BLINK_CUSTOM, 100, 100);
  } else if (event_base == WIFI_EVENT &&
             event_id == WIFI_EVENT_STA_DISCONNECTED) {
    ESP_LOGI(TAG, "Disconnected. Reconnecting...");
    xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
    esp_wifi_connect();

    // Slow blink while disconnected
    led_indicator_control(LED_CMD_BLINK_CUSTOM, 500, 500);
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
    xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);

    // Solid LED ON when connected
    led_indicator_control(LED_CMD_SOLID_ON, 0, 0);
  }
}

// === WIFI INIT ===
void wifi_init_sta(void) {
  wifi_event_group = xEventGroupCreate();

  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                             &wifi_event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                             &wifi_event_handler, NULL));

  ESP_ERROR_CHECK(esp_netif_init());

  esp_netif_t *netif = esp_netif_create_default_wifi_sta();
  ESP_ERROR_CHECK(esp_netif_set_hostname(netif, "xense"));

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(
      esp_wifi_set_config(WIFI_IF_STA, (wifi_config_t *)&sta_config));

  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI(TAG, "Wi-Fi configured and started.");
}
