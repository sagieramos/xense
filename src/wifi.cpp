#include "config_xense.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "xense_utils.h"

#define TAG "WiFiModule"
#define DEFAULT_SCAN_LIST_SIZE 20

#define WIFI_CONNECTED_BIT BIT0
static EventGroupHandle_t wifi_event_group;
bool is_connected = false;

Xense_Station nvs_sta;

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

wifi_config_t ap_config = {
    .ap = {.ssid = "xense_ap",
           .password = "81286389",
           .ssid_len = 0, // 0 = use strlen of ssid
           .channel = 1,
           .authmode = WIFI_AUTH_WPA2_PSK,
           .ssid_hidden = 1, // visible network
           .max_connection = 4,
           .beacon_interval = 100, // in TUs, 102.4ms
           .csa_count = 3,         // standard default
           .dtim_period = 1,       // default DTIM
           .pairwise_cipher = WIFI_CIPHER_TYPE_CCMP,
           .ftm_responder = false,
           .pmf_cfg = {.capable = true, .required = false},
           .sae_pwe_h2e = WPA3_SAE_PWE_BOTH}};

// === WIFI EVENT HANDLER ===
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data) {
  if (event_base == WIFI_EVENT) {
    switch (event_id) {
    case WIFI_EVENT_STA_START:
      LOG_XENSE(TAG, "Wi-Fi started. Connecting...");
      esp_wifi_connect();
      led_indicator_control(LED_CMD_BLINK_CUSTOM, 100, 100); // Fast blink
      break;

    case WIFI_EVENT_STA_DISCONNECTED:
      LOG_XENSE(TAG, "Disconnected. Reconnecting...");
      xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
      esp_wifi_connect();
      is_connected = false;
      led_indicator_control(LED_CMD_BLINK_CUSTOM, 500, 500); // Slow blink
      break;

    case WIFI_EVENT_AP_START:
      LOG_XENSE(TAG, "Access Point started");
      break;

    case WIFI_EVENT_AP_STACONNECTED: {
      wifi_event_ap_staconnected_t *event =
          (wifi_event_ap_staconnected_t *)event_data;
      LOG_XENSE(TAG, "Station connected, MAC: " MACSTR ", AID: %d",
                MAC2STR(event->mac), event->aid);
      is_connected = true;
      break;
    }

    case WIFI_EVENT_STA_CONNECTED: {
      wifi_event_sta_connected_t *event =
          (wifi_event_sta_connected_t *)event_data;
      LOG_XENSE(TAG, "Station connected to AP, SSID: %s, BSSID: " MACSTR,
                event->ssid, MAC2STR(event->bssid));
      is_connected = true;
      break;
    }

    case WIFI_EVENT_AP_STADISCONNECTED: {
      wifi_event_ap_stadisconnected_t *event =
          (wifi_event_ap_stadisconnected_t *)event_data;
      xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
      LOG_XENSE(TAG, "Station disconnected, MAC: " MACSTR ", AID: %d",
                MAC2STR(event->mac), event->aid);
      break;
    }

    case WIFI_EVENT_SCAN_DONE:
      LOG_XENSE(TAG, "Wi-Fi scan completed");
      // EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
      // WIFI_CONNECTED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
      if (xEventGroupGetBits(wifi_event_group) & WIFI_CONNECTED_BIT) {
        led_indicator_control(LED_CMD_SOLID_ON, 0, 0); // Solid ON
      } else {
        led_indicator_control(LED_CMD_BLINK_CUSTOM, 500, 500); // Slow blink
      }
      xEventGroupSetBits(wifi_event_group, BIT0); // Set scan done bit
      break;
    }

  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    LOG_XENSE(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
    xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    esp_wifi_set_mode(WIFI_MODE_APSTA);
    led_indicator_control(LED_CMD_SOLID_ON, 0, 0); // Solid ON
  }
}

// === WIFI INIT ===
void wifi_init_sta(void) {
  wifi_event_group = xEventGroupCreate();

  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_START,
                                             &wifi_event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(
      WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &wifi_event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_SCAN_DONE,
                                             &wifi_event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                             &wifi_event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, WIFI_EVENT_STA_CONNECTED,
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

  LOG_XENSE(TAG, "Wi-Fi configured and started.");
}

void wifi_init_ap() {
  wifi_event_group = xEventGroupCreate();

  ESP_ERROR_CHECK(esp_netif_init());
  esp_netif_create_default_wifi_ap();

  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_START,
                                             &wifi_event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(
      WIFI_EVENT, WIFI_EVENT_AP_STACONNECTED, &wifi_event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(
      WIFI_EVENT, WIFI_EVENT_AP_STADISCONNECTED, &wifi_event_handler, NULL));

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  if (strlen((char *)ap_config.ap.password) == 0) {
    ap_config.ap.authmode = WIFI_AUTH_OPEN;
  }

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  LOG_XENSE("WiFiModule", "AP started. SSID:%s password:%s", ap_config.ap.ssid,
            ap_config.ap.password);
}

// AP and STA mode

void wifi_init_ap_sta() {
  // Create event group for Wi-Fi events
  wifi_event_group = xEventGroupCreate();

  // Initialize TCP/IP stack (this also creates the default event loop)
  ESP_ERROR_CHECK(esp_netif_init());

  // Create network interfaces
  esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
  esp_netif_create_default_wifi_ap();

  // Register only necessary events
  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_START,
                                             &wifi_event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(
      WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &wifi_event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_SCAN_DONE,
                                             &wifi_event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_START,
                                             &wifi_event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(
      WIFI_EVENT, WIFI_EVENT_AP_STACONNECTED, &wifi_event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(
      WIFI_EVENT, WIFI_EVENT_AP_STADISCONNECTED, &wifi_event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                             &wifi_event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, WIFI_EVENT_STA_CONNECTED,
                                             &wifi_event_handler, NULL));

  // Set hostname for STA interface (optional: can also set for AP if needed)
  set_xense_hostname(sta_netif);
  set_xense_ap_ssid(&ap_config);

  // ESP_ERROR_CHECK(esp_netif_set_hostname(sta_netif, "xense"));

  // Initialize Wi-Fi driver
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));

  // Set Wi-Fi configurations
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));

  if (load_station_config(&nvs_sta) == ESP_OK) {
    LOG_XENSE(TAG, "Loaded station config from NVS");
    strncpy((char *)sta_config.ssid, (char *)nvs_sta.ssid,
            sizeof(sta_config.ssid));
    strncpy((char *)sta_config.password, (char *)nvs_sta.password,
            sizeof(sta_config.password));
  } else {
    LOG_XENSE(TAG, "Failed to load station config from NVS");
  }

  ESP_ERROR_CHECK(
      esp_wifi_set_config(WIFI_IF_STA, (wifi_config_t *)&sta_config));

  // Start Wi-Fi
  ESP_ERROR_CHECK(esp_wifi_start());

  // Initiate connection for STA
  ESP_ERROR_CHECK(esp_wifi_connect());

  LOG_XENSE(TAG, "AP and STA mode started.");

  vTaskDelay(pdMS_TO_TICKS(1000));
  wifi_scan();
}

void wifi_scan() {
#ifdef LOG_ENABLED
  wifi_mode_t mode;
  esp_err_t err = esp_wifi_get_mode(&mode);
  if (err != ESP_OK) {
    LOG_XENSE(TAG, "Failed to get Wi-Fi mode");
    return;
  }
  if (mode != WIFI_MODE_STA && mode != WIFI_MODE_APSTA) {
    LOG_XENSE(TAG, "Wi-Fi mode is not STA or APSTA");
    return;
  }
#endif

  if (!is_connected) {
    LOG_XENSE(TAG, "Wi-Fi not connected, cannot scan");
    esp_wifi_disconnect();
  }

  uint16_t number = DEFAULT_SCAN_LIST_SIZE;
  wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
  uint16_t ap_count = 0;
  memset(ap_info, 0, sizeof(ap_info));

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_start());

  wifi_scan_config_t *scan_config =
      (wifi_scan_config_t *)calloc(1, sizeof(wifi_scan_config_t));
  if (!scan_config) {
    LOG_XENSE(TAG, "Memory Allocation for scan config failed!");
    return;
  }
  ESP_ERROR_CHECK(esp_wifi_scan_start(NULL, true));

  vTaskDelay(pdMS_TO_TICKS(1000));

  // Get scan results
  ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
  ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));

  // Process results
  for (int i = 0; i < number; i++) {
    LOG_XENSE(TAG, "-------------------%d---------------------", 1 + i);
    LOG_XENSE(TAG, "SSID: %s", ap_info[i].ssid);
    LOG_XENSE(TAG, "BSSID: " MACSTR, MAC2STR(ap_info[i].bssid));
    LOG_XENSE(TAG, "RSSI: %d", ap_info[i].rssi);
    LOG_XENSE(TAG, "Auth: %d", ap_info[i].authmode);
    LOG_XENSE(TAG, "Channel: %d", ap_info[i].primary);
    LOG_XENSE(TAG, "WiFi Cipher: %d", ap_info[i].pairwise_cipher);
    LOG_XENSE("Chennel Width or Bandwidth", " %d", ap_info[i].bandwidth);
  }

  free(scan_config);

  LOG_XENSE(TAG, "------------Scan Completed(%d)-----------", ap_count);

  if (!is_connected) {
    ESP_ERROR_CHECK(esp_wifi_connect());
  }
}
