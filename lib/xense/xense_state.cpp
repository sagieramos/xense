#include "./xense.h"

/**
 * @brief Serialize the Xense state structure into a buffer.
 *
 * @param state The Xense state structure to serialize.
 * @param buffer The buffer to serialize the state into.
 * @param buffer_size The size of the buffer.
 * @param type_id The type ID to prepend to the buffer.
 * @param callback The callback to call after serialization.
 *
 * @return XENSE_OK if the serialization was successful, an error code
 * otherwise.
 */

NpbStatus serialize_xense_state(const Xense_state &state, uint8_t *buffer,
                                size_t *buffer_size, uint8_t type_id,
                                uint8_t *mac_address, xense_callback callback) {
  return npb_encode({&state, Xense_state_fields}, buffer, buffer_size, type_id,
                    mac_address, callback);
}

/**
 * @brief Deserialize the Xense state structure from a buffer.
 *
 * @param buffer The buffer to deserialize the state from.
 * @param buffer_size The size of the buffer.
 * @param state The Xense state structure to deserialize into.
 * @param callback The callback to call after serialization.
 *
 * @return True if the deserialization was successful, false otherwise.
 */
NpbStatus deserialize_xense_state(const uint8_t *buffer, size_t buffer_size,
                                  Xense_state &state, xense_callback callback) {
  return npb_decode({&state, Xense_state_fields}, buffer, buffer_size,
                    callback);
}