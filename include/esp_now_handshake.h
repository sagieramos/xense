/**
 * @file esp_now_handshake.h
 * @brief Header file for ESP-NOW peer management with handshake verification
 */

 #ifndef __ESP_NOW_HANDSHAKE_H__
 #define __ESP_NOW_HANDSHAKE_H__
 
 #include "esp_now.h"
 
 #ifdef __cplusplus
 extern "C" {
 #endif
 
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
 esp_err_t esp_now_handshake_init(void);
 
 /**
  * @brief Add a peer with handshake verification
  * 
  * This function adds a peer to the ESP-NOW peer list and initiates a handshake.
  * The peer will be removed if the handshake does not complete within the timeout period.
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
 esp_err_t esp_now_add_peer_with_handshake(const esp_now_peer_info_t *peer);
 
 /**
  * @brief Deinitialize the handshake module
  * 
  * @return esp_err_t 
  *         - ESP_OK: Success
  */
 esp_err_t esp_now_handshake_deinit(void);
 
 #ifdef __cplusplus
 }
 #endif
 
 #endif /* __ESP_NOW_HANDSHAKE_H__ */