#include "../include/xense.h"

/**
 * Serialize the Xense state structure into a buffer.
 *
 * @param state The Xense state structure to serialize.
 * @param buffer The buffer to serialize the data into.
 * @param buffer_size The size of the buffer.
 * @param type_id The type ID to prepend to the buffer.
 * @param callback The callback to call after serialization.
 * @return True if the serialization was successful, false otherwise.
 */
bool serialize_xense_state(const Xense_state &state, uint8_t *buffer,
                           size_t *buffer_size, uint8_t type_id,
                           xense_data_callback callback) {
  if (*buffer_size < 1) {
    return false; // Buffer too small to hold type ID
  }
  buffer[0] = type_id;
  pb_ostream_t stream = pb_ostream_from_buffer(buffer + 1, *buffer_size - 1);
  if (!pb_encode(&stream, Xense_state_fields, &state)) {
    return false;
  }
  if (callback) {
    callback(buffer, stream.bytes_written);
  }
  return true;
}

/**
 * Deserialize the Xense state structure from a buffer.
 *
 * @param buffer The buffer to deserialize the data from.
 * @param buffer_size The size of the buffer.
 * @param state The Xense state structure to deserialize into.
 * @return True if the deserialization was successful, false otherwise.
 */
bool deserialize_xense_state(const uint8_t *buffer, size_t buffer_size,
                             Xense_state &state) {
  if (buffer_size < 1) {
    return false; // Buffer too small to contain type ID
  }
  pb_istream_t stream = pb_istream_from_buffer(buffer + 1, buffer_size - 1);
  if (!pb_decode(&stream, Xense_state_fields, &state)) {
    return false;
  }
  return true;
}