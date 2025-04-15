#include "../include/xense.h"

/**
 * Serialize the Xense metadata structure into a buffer.
 *
 * @param metadata The Xense metadata structure to serialize.
 * @param buffer The buffer to serialize the metadata into.
 * @param buffer_size The size of the buffer.
 * @param type_id The type ID to prepend to the buffer.
 * @param callback The callback to call after serialization.
 * @return True if the serialization was successful, false otherwise.
 */

bool serialize_metadata(const Xense_metadata &metadata, uint8_t *buffer,
                        size_t *buffer_size, uint8_t type_id,
                        xense_data_callback callback) {
  if (*buffer_size < 1) {
    return false; // Buffer too small to hold type ID
  }
  buffer[0] = type_id;
  pb_ostream_t stream = pb_ostream_from_buffer(buffer + 1, *buffer_size - 1);
  if (!pb_encode(&stream, Xense_metadata_fields, &metadata)) {
    return false;
  }
  if (callback) {
    callback(buffer, stream.bytes_written);
  }
  return true;
}
/**
 * Deserialize the Xense metadata structure from a buffer.
 *
 * @param buffer The buffer to deserialize the metadata from.
 * @param buffer_size The size of the buffer.
 * @param metadata The Xense metadata structure to deserialize into.
 * @return True if the deserialization was successful, false otherwise.
 */
bool deserialize_metadata(const uint8_t *buffer, size_t buffer_size,
                          Xense_metadata &metadata) {
  if (buffer_size < 1) {
    return false; // Buffer too small to contain type ID
  }
  pb_istream_t stream = pb_istream_from_buffer(buffer + 1, buffer_size - 1);
  if (!pb_decode(&stream, Xense_metadata_fields, &metadata)) {
    return false;
  }
  return true;
}