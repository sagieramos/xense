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

XenseStatus serialize_xense_state(const Xense_state &state, uint8_t *buffer,
                                  size_t *buffer_size, uint8_t type_id,
                                  xense_callback callback) {
  return serialize_xense_message({&state, Xense_state_fields}, buffer,
                                 buffer_size, type_id, callback);
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
XenseStatus deserialize_xense_state(const uint8_t *buffer, size_t buffer_size,
                                    Xense_state &state,
                                    xense_callback callback) {
  return deserialize_xense_message({&state, Xense_state_fields}, buffer,
                                   buffer_size, callback);
}