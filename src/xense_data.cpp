#include "../include/xense.h"

/**
 * Serialize the Xense data structure into a buffer.
 *
 * @param data The Xense data structure to serialize.
 * @param buffer The buffer to serialize the data into.
 * @param buffer_size The size of the buffer.
 * @param type_id The type ID to prepend to the buffer.
 * @param callback The callback to call after serialization.
 * @return True if the serialization was successful, false otherwise.
 */

bool serialize_xense_data(const Xense_data &data, uint8_t *buffer,
                          size_t *buffer_size, uint8_t type_id,
                          xense_data_callback callback) {
  buffer[0] = type_id;
  pb_ostream_t stream = pb_ostream_from_buffer(buffer + 1, *buffer_size - 1);
  if (!pb_encode(&stream, Xense_data_fields, &data)) {
    return false;
  }
  if (callback) {
    callback(buffer, stream.bytes_written);
  }
  return true;
}

/**
 * Deserialize the Xense data structure from a buffer.
 *
 * @param buffer The buffer to deserialize the data from.
 * @param buffer_size The size of the buffer.
 * @param data The Xense data structure to deserialize into.
 * @return True if the deserialization was successful, false otherwise.
 */

bool deserialize_xense_data(const uint8_t *buffer, size_t buffer_size,
                            Xense_data &data) {
  pb_istream_t stream = pb_istream_from_buffer(buffer + 1, buffer_size - 1);
  if (!pb_decode(&stream, Xense_data_fields, &data)) {
    return false;
  }
  return true;
}