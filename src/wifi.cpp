#include "config_xense.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "xense_utils.h"

#define TAG "WiFiModule"

#define WIFI_CONNECTED_BIT BIT0
static EventGroupHandle_t wifi_event_group;

// === LED BLINK ===
static const wifi_sta_config_t sta_config = {
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
           .password = "8128638936",
           .ssid_len = 0, // 0 = use strlen of ssid
           .channel = 1,
           .authmode = WIFI_AUTH_WPA2_PSK,
           .ssid_hidden = 0, // visible network
           .max_connection = 4,
           .beacon_interval = 100, // in TUs, 102.4ms
           .csa_count = 3,         // standard default
           .dtim_period = 1,       // default DTIM
           .pairwise_cipher = WIFI_CIPHER_TYPE_CCMP,
           .ftm_responder = false,
           .pmf_cfg = {.capable = true, .required = false},
           .sae_pwe_h2e = WPA3_SAE_PWE_BOTH}};

wifi_scan_config_t scan_config = {
    .ssid = NULL,        // NULL: scan all SSIDs
    .bssid = NULL,       // NULL: scan all BSSIDs
    .channel = 0,        // 0: scan all channels (or use channel_bitmap)
    .show_hidden = true, // true: include hidden SSIDs
    .scan_type = WIFI_SCAN_TYPE_ACTIVE, // or WIFI_SCAN_TYPE_PASSIVE
    .scan_time =
        {
            .active =
                {
                    .min = 100, // WIFI_ACTIVE_SCAN_MIN_DEFAULT_TIME,
                    .max = 300  // WIFI_ACTIVE_SCAN_MAX_DEFAULT_TIME
                },
            .passive = 300 // WIFI_PASSIVE_SCAN_DEFAULT_TIME
        },                
    .home_chan_dwell_time = WIFI_SCAN_HOME_CHANNEL_DWELL_DEFAULT_TIME, // 30ms
    .channel_bitmap = {
        .ghz_2_channels =
            0x1FFF, // Enable channels 1–13 (bits 0–12 = channels 1–13)
        .ghz_5_channels = 0 // Skip 5 GHz
    }};

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
  esp_netif_t *ap_netif = esp_netif_create_default_wifi_ap();

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

  // Set hostname for STA interface (optional: can also set for AP if needed)
  ESP_ERROR_CHECK(esp_netif_set_hostname(sta_netif, "xense"));

  // Register event handlers BEFORE starting Wi-Fi
  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                             &wifi_event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                             &wifi_event_handler, NULL));

  // Initialize Wi-Fi driver
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  // Configure Wi-Fi mode
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));

  // Set Wi-Fi configurations
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));
  ESP_ERROR_CHECK(
      esp_wifi_set_config(WIFI_IF_STA, (wifi_config_t *)&sta_config));

  // Start Wi-Fi
  ESP_ERROR_CHECK(esp_wifi_start());

  // Initiate connection for STA
  ESP_ERROR_CHECK(esp_wifi_connect());

  LOG_XENSE(TAG, "AP and STA mode started.");
}

// === Scan APs ===
esp_err_t wifi_scan() {
  // Clear previous scan event
  xEventGroupClearBits(wifi_event_group, BIT0);

  // Start the scan
  ESP_LOGI(TAG, "Starting Wi-Fi scan...");
  esp_err_t ret = esp_wifi_scan_start(&scan_config, false); // Non-blocking scan
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to start scan: %s", esp_err_to_name(ret));
    return ret;
  }

  // Wait for scan to complete (timeout after 10 seconds)
  EventBits_t bits = xEventGroupWaitBits(wifi_event_group, BIT0, pdTRUE,
                                         pdFALSE, pdMS_TO_TICKS(10000));
  if (!(bits & BIT0)) {
    ESP_LOGE(TAG, "Scan timed out");
    return ESP_ERR_TIMEOUT;
  }

  // Retrieve scan results
  uint16_t ap_count = 0;
  esp_wifi_scan_get_ap_num(&ap_count);
  ESP_LOGI(TAG, "Found %u access points", ap_count);

  if (ap_count == 0) {
    return ESP_OK;
  }

  wifi_ap_record_t *ap_list =
      (wifi_ap_record_t *)malloc(ap_count * sizeof(wifi_ap_record_t));
  if (ap_list == NULL) {
    ESP_LOGE(TAG, "Failed to allocate memory for AP list");
    return ESP_ERR_NO_MEM;
  }

  ret = esp_wifi_scan_get_ap_records(&ap_count, ap_list);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to get AP records: %s", esp_err_to_name(ret));
    free(ap_list);
    return ret;
  }

  // Print scan results
  for (uint16_t i = 0; i < ap_count; i++) {
    wifi_ap_record_t *ap = &ap_list[i];
    ESP_LOGI(TAG, "AP %u: SSID='%s', RSSI=%d, Channel=%u, Auth=%d", i + 1,
             ap->ssid, ap->rssi, ap->primary, ap->authmode);
  }

  // Clean up
  free(ap_list);
  return ESP_OK;
}
