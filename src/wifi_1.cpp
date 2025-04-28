#include "custom_wifi.h"
#include "esp_mac.h"
#include "log.h"
#include <cstring>

void set_xense_hostname(esp_netif_t *netif) {
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA); // Read the MAC address

  uint32_t mac_decimal = (mac[3] << 16) | (mac[4] << 8) | (mac[5]);

  char hostname[32];
  snprintf(hostname, sizeof(hostname), "xense%lu", (unsigned long)mac_decimal);

  esp_netif_set_hostname(netif, hostname);

  LOG_XENSE("HOSTNAME", "Set hostname to: %s", hostname);
}

void set_xense_ap_ssid(wifi_config_t *ap_config) {
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA); // Read the STA MAC

  uint32_t mac_decimal = (mac[3] << 16) | (mac[4] << 8) | (mac[5]);

  snprintf((char *)ap_config->ap.ssid, sizeof(ap_config->ap.ssid),
           "xense%lu_ap", (unsigned long)mac_decimal);

  ap_config->ap.ssid_len =
      strlen((char *)ap_config->ap.ssid); // Ensure length is set correctly

  LOG_XENSE("AP_SSID", "Updated AP SSID to: %s (Length: %d)",
            ap_config->ap.ssid, ap_config->ap.ssid_len);
}
