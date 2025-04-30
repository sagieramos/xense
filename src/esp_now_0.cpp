#include "config_xense.h"
#include "custom_esp_now.h"
#include "esp_log.h"

peer_entry_t peer_list[MAX_PEERS];

const uint8_t lmk[ESP_NOW_KEY_LEN] = LMK_KEY;

void show_esp_now_peers(void) {
    // Get the total number of peers
    esp_now_peer_num_t peer_num = {
        .total_num = 0,     
        .encrypt_num = 0
    };
    
    esp_err_t result = esp_now_get_peer_num(&peer_num);
    if (result != ESP_OK) {
      ESP_LOGE(ESP_NOW_TAG, "Failed to get peer number: %s", esp_err_to_name(result));
      return;
    }
  
    ESP_LOGI(ESP_NOW_TAG, "ESP-NOW Peer List: Total: %d, Encrypted: %d\n",
             peer_num.total_num, peer_num.encrypt_num);
  
    // If no peers, return early
    if (peer_num.total_num == 0) {
      ESP_LOGI(ESP_NOW_TAG, "No peers added.\n");
      return;
    }
  
    // Iterate through all peers
    esp_now_peer_info_t peer;
    bool from_head = true; // Start from the head of the list
  
    for (int i = 0; i < peer_num.total_num; i++) {
      // Get the next peer from the list
      result = esp_now_fetch_peer(from_head, &peer);
      from_head = false; // After first fetch, continue from where we left off
  
      if (result == ESP_OK) {
        ESP_LOGI(ESP_NOW_TAG, "Peer %d: MAC: " MACSTR, i + 1, MAC2STR(peer.peer_addr));
  
        ESP_LOGI(ESP_NOW_TAG, "Channel: %d, Interface: %d, Encrypted: %s\n", 
                 peer.channel, peer.ifidx, peer.encrypt ? "Yes" : "No");
      } else {
        ESP_LOGW(ESP_NOW_TAG, "Failed to fetch peer %d, error: %d\n", i + 1, result);
      }
    }
  }
  