/**
 * @file esp_now_add_with_handshake.c
 * @brief Implementation of peer management with handshake verification
 */

#include "esp_log.h"
#include "esp_now.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include <string.h>

#define TAG "ESP_NOW_HANDSHAKE"
#define HANDSHAKE_TIMEOUT_MS 3000 // Timeout for handshake in milliseconds
#define HANDSHAKE_MSG_TYPE 0x01   // Message type for handshake messages

// Handshake protocol message structure
typedef struct {
  uint8_t type;   // Message type (0x01 for handshake)
  uint8_t seq;    // Sequence number
  uint8_t status; // Status (0x01: request, 0x02: ack)
} handshake_msg_t;

// Structure to track pending handshakes
typedef struct {
  uint8_t peer_addr[ESP_NOW_ETH_ALEN]; // Peer MAC address
  TimerHandle_t timer;                 // Timeout timer
  bool handshake_complete;             // Flag to track if handshake completed
} handshake_peer_t;

// Global data
static handshake_peer_t *pending_handshake = NULL;
static esp_now_send_cb_t original_send_cb = NULL;
static esp_now_recv_cb_t original_recv_cb = NULL;

// Forward declarations
static void handshake_send_cb(const uint8_t *mac_addr,
                              esp_now_send_status_t status);
static void handshake_recv_cb(const esp_now_recv_info_t *esp_now_info,
                              const uint8_t *data, int data_len);
static void handshake_timeout_cb(TimerHandle_t timer);

/**
 * @brief Add a peer with handshake verification
 *
 * This function adds a peer to the ESP-NOW peer list and initiates a handshake.
 * The peer will be removed if the handshake does not complete within the
 * timeout period.
 *
 * @param peer Peer information
 * @return esp_err_t
 *         - ESP_OK: Success
 *         - ESP_ERR_ESPNOW_NOT_INIT: ESP-NOW not initialized
 *         - ESP_ERR_ESPNOW_ARG: Invalid argument
 *         - ESP_ERR_ESPNOW_FULL: Peer list is full
 *         - ESP_ERR_ESPNOW_NO_MEM: Out of memory
 *         - ESP_ERR_ESPNOW_EXIST: Peer already exists
 *         - Others: Failed to add peer
 */
esp_err_t esp_now_add_peer_with_handshake(const esp_now_peer_info_t *peer) {
  esp_err_t ret;

  // Check if ESP-NOW is initialized
  if (pending_handshake != NULL) {
    ESP_LOGE(TAG, "Another handshake is already in progress");
    return ESP_ERR_ESPNOW_INTERNAL;
  }

  // Add the peer
  ret = esp_now_add_peer(peer);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to add peer: %d", ret);
    return ret;
  }

  // Allocate memory for the handshake state
  pending_handshake = (handshake_peer_t *)malloc(sizeof(handshake_peer_t));
  if (pending_handshake == NULL) {
    ESP_LOGE(TAG, "Failed to allocate memory for handshake");
    esp_now_del_peer(peer->peer_addr);
    return ESP_ERR_ESPNOW_NO_MEM;
  }

  // Initialize handshake state
  memcpy(pending_handshake->peer_addr, peer->peer_addr, ESP_NOW_ETH_ALEN);
  pending_handshake->handshake_complete = false;

  // Create timer for handshake timeout
  pending_handshake->timer =
      xTimerCreate("handshake_timer", pdMS_TO_TICKS(HANDSHAKE_TIMEOUT_MS),
                   pdFALSE, // One-shot timer
                   pending_handshake, handshake_timeout_cb);

  if (pending_handshake->timer == NULL) {
    ESP_LOGE(TAG, "Failed to create handshake timer");
    free(pending_handshake);
    pending_handshake = NULL;
    esp_now_del_peer(peer->peer_addr);
    return ESP_ERR_ESPNOW_NO_MEM;
  }

  // Save original callbacks
  esp_now_register_send_cb(handshake_send_cb);
  esp_now_register_recv_cb(handshake_recv_cb);

  // Prepare handshake message
  handshake_msg_t msg;
  msg.type = HANDSHAKE_MSG_TYPE;
  msg.seq = 0;
  msg.status = 0x01; // Request

  // Send handshake request
  ret = esp_now_send(peer->peer_addr, (const uint8_t *)&msg, sizeof(msg));
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to send handshake request: %d", ret);
    xTimerDelete(pending_handshake->timer, 0);
    free(pending_handshake);
    pending_handshake = NULL;
    esp_now_del_peer(peer->peer_addr);
    return ret;
  }

  // Start timeout timer
  if (xTimerStart(pending_handshake->timer, 0) != pdPASS) {
    ESP_LOGE(TAG, "Failed to start handshake timer");
    xTimerDelete(pending_handshake->timer, 0);
    free(pending_handshake);
    pending_handshake = NULL;
    esp_now_del_peer(peer->peer_addr);
    return ESP_ERR_ESPNOW_INTERNAL;
  }

  ESP_LOGI(TAG, "Handshake initiated with peer: %02x:%02x:%02x:%02x:%02x:%02x",
           peer->peer_addr[0], peer->peer_addr[1], peer->peer_addr[2],
           peer->peer_addr[3], peer->peer_addr[4], peer->peer_addr[5]);

  return ESP_OK;
}

/**
 * @brief Send callback for handshake
 */
static void handshake_send_cb(const uint8_t *mac_addr,
                              esp_now_send_status_t status) {
  // Call original callback if set
  if (original_send_cb != NULL) {
    original_send_cb(mac_addr, status);
  }

  // Check if this is the peer we're waiting for
  if (pending_handshake != NULL &&
      memcmp(mac_addr, pending_handshake->peer_addr, ESP_NOW_ETH_ALEN) == 0) {

    if (status == ESP_NOW_SEND_FAIL) {
      ESP_LOGW(TAG, "Handshake message send failed");
      // We'll let the timeout handle this failure
    }
  }
}

/**
 * @brief Receive callback for handshake
 */
static void handshake_recv_cb(const esp_now_recv_info_t *esp_now_info,
                              const uint8_t *data, int data_len) {
  // Call original callback if set
  if (original_recv_cb != NULL) {
    original_recv_cb(esp_now_info, data, data_len);
  }
  // Process handshake messages
  if (pending_handshake != NULL && data_len >= sizeof(handshake_msg_t)) {
    handshake_msg_t *msg = (handshake_msg_t *)data;
    // Check if this is a handshake message from the peer we're waiting for
    if (msg->type == HANDSHAKE_MSG_TYPE &&
        memcmp(esp_now_info->src_addr, pending_handshake->peer_addr,
               ESP_NOW_ETH_ALEN) == 0) {

      if (msg->status == 0x02) { // ACK
        ESP_LOGI(
            TAG, "Handshake completed with peer: %02X:%02X:%02X:%02X:%02X:%02X",
            pending_handshake->peer_addr[0], pending_handshake->peer_addr[1],
            pending_handshake->peer_addr[2], pending_handshake->peer_addr[3],
            pending_handshake->peer_addr[4], pending_handshake->peer_addr[5]);

        // Handshake completed successfully
        pending_handshake->handshake_complete = true;

        // Stop the timer
        xTimerStop(pending_handshake->timer, 0);
        xTimerDelete(pending_handshake->timer, 0);

        // Clean up
        free(pending_handshake);
        pending_handshake = NULL;

        // Restore original callbacks
        esp_now_register_send_cb(original_send_cb);
        esp_now_register_recv_cb(original_recv_cb);
      } else if (msg->status == 0x01) { // Request
        // Send ACK response
        handshake_msg_t response;
        response.type = HANDSHAKE_MSG_TYPE;
        response.seq = msg->seq;
        response.status = 0x02; // ACK

        esp_err_t ret =
            esp_now_send(esp_now_info->src_addr, (const uint8_t *)&response,
                         sizeof(response));
        if (ret != ESP_OK) {
          ESP_LOGE(TAG, "Failed to send handshake ACK: %d", ret);
        }
      }
    }
  }
}

/**
 * @brief Timeout callback for handshake
 */
static void handshake_timeout_cb(TimerHandle_t timer) {
  if (pending_handshake != NULL) {
    if (!pending_handshake->handshake_complete) {
      ESP_LOGW(
          TAG, "Handshake timeout with peer: %02X:%02X:%02X:%02X:%02X:%02X",
          pending_handshake->peer_addr[0], pending_handshake->peer_addr[1],
          pending_handshake->peer_addr[2], pending_handshake->peer_addr[3],
          pending_handshake->peer_addr[4], pending_handshake->peer_addr[5]);

      // Remove the peer as handshake failed
      esp_now_del_peer(pending_handshake->peer_addr);

      // Clean up
      free(pending_handshake);
      pending_handshake = NULL;

      // Restore original callbacks
      esp_now_register_send_cb(original_send_cb);
      esp_now_register_recv_cb(original_recv_cb);
    }
  }
}

/**
 * @brief Initialize the handshake module
 *
 * This function should be called after esp_now_init() but before
 * using esp_now_add_peer_with_handshake().
 *
 * @return esp_err_t
 *         - ESP_OK: Success
 *         - ESP_ERR_ESPNOW_NOT_INIT: ESP-NOW not initialized
 */
esp_err_t esp_now_handshake_init(void) {
  // Store original callbacks
  esp_err_t ret = esp_now_register_send_cb(NULL);
  if (ret != ESP_OK) {
    return ret;
  }

  ret = esp_now_register_recv_cb(NULL);
  if (ret != ESP_OK) {
    return ret;
  }

  // Get original callbacks
  original_send_cb = NULL;
  original_recv_cb = NULL;

  ESP_LOGI(TAG, "Handshake module initialized");
  return ESP_OK;
}

/**
 * @brief Deinitialize the handshake module
 *
 * @return esp_err_t
 *         - ESP_OK: Success
 */
esp_err_t esp_now_handshake_deinit(void) {
  // Clean up any ongoing handshake
  if (pending_handshake != NULL) {
    xTimerStop(pending_handshake->timer, 0);
    xTimerDelete(pending_handshake->timer, 0);
    free(pending_handshake);
    pending_handshake = NULL;
  }

  // Restore original callbacks
  esp_now_register_send_cb(original_send_cb);
  esp_now_register_recv_cb(original_recv_cb);

  original_send_cb = NULL;
  original_recv_cb = NULL;

  ESP_LOGI(TAG, "Handshake module deinitialized");
  return ESP_OK;
}