#include "./xense.h"

const int SCALE_FACTOR = 1e9;

/**
 * Serialize the Xense coordinate structure into a buffer.
 *
 * @param coordinate The Xense coordinate structure to serialize.
 * @param buffer The buffer to serialize the coordinate into.
 * @param buffer_size The size of the buffer.
 * @param type_id The type ID to prepend to the buffer.
 * @param callback The callback to call after serialization.
 * @return True if the serialization was successful, false otherwise.
 */

 XenseStatus serialize_xense_gps(const Xense_gps &gps,
   uint8_t *buffer, size_t *buffer_size,
   uint8_t type_id, xense_callback callback) {
  return serialize_xense_message({&gps, Xense_gps_fields},
    buffer, buffer_size, type_id, callback);
}
/**
 * Deserialize the Xense coordinate structure from a buffer.
 *
 * @param buffer The buffer to deserialize the coordinate from.
 * @param buffer_size The size of the buffer.
 * @param coordinate The Xense coordinate structure to deserialize into.
 * @param callback The callback to call after serialization.
 * @return True if the deserialization was successful, false otherwise.
 */
XenseStatus deserialize_xense_coordinate(const uint8_t *buffer, size_t buffer_size,
  Xense_gps &gps,
  xense_callback callback) {
  return deserialize_xense_message({&gps, Xense_gps_fields},
    buffer, buffer_size, callback);
}