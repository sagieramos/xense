#include "./xense.h"

const int SCALE_FACTOR = 1e9;

/**
 * @brief Serialize the Xense coordinate structure into a buffer.
 *
 * @param coordinate The Xense coordinate structure to serialize.
 * @param buffer The buffer to serialize the coordinate into.
 * @param buffer_size The size of the buffer.
 * @param type_id The type ID to prepend to the buffer.
 * @param callback The callback to call after serialization.
 * @return True if the serialization was successful, false otherwise.
 */

NpbStatus serialize_xense_gps(const Xense_gps &gps, uint8_t *buffer,
                              size_t *buffer_size, uint8_t type_id,
                              uint8_t *mac_address, xense_callback callback) {
  return npb_encode({&gps, Xense_gps_fields}, buffer, buffer_size, type_id,
                    mac_address, callback);
}
/**
 * @brief Deserialize the Xense coordinate structure from a buffer.
 *
 * @param buffer The buffer to deserialize the coordinate from.
 * @param buffer_size The size of the buffer.
 * @param coordinate The Xense coordinate structure to deserialize into.
 * @param callback The callback to call after serialization.
 * @return True if the deserialization was successful, false otherwise.
 */
NpbStatus deserialize_xense_gps(const uint8_t *buffer, size_t buffer_size,
                                Xense_gps &gps, xense_callback callback) {
  return npb_decode({&gps, Xense_gps_fields}, buffer, buffer_size, callback);
}