#include "./xense.h"
/* #include <TinyGPSPlus.h> */

const int SCALE_FACTOR = 1e6;

/* TinyGPSPlus gps;

void testinggps() {
    // Simulate GPS data
    gps.location.rawLat();
    gps.altitude.meters();
    gps.location.rawLng();
    gps.satellites.value();
    gps.date.value();
    gps.time.value();
    gps.hdop.value();
}
 */
/**
 * Serialize the Xense data structure into a buffer.
 *
 * @param data The Xense data structure to serialize.
 * @param buffer The buffer to serialize the data into.
 * @param buffer_size The size of the buffer.
 * @param type_id The type ID to prepend to the buffer.
 * @param callback The callback to call after serialization.
 * @return XENSE_OK if the serialization was successful, an error code otherwise.
 */

XenseStatus serialize_xense_data(const Xense_data &data,
  uint8_t *buffer, size_t *buffer_size,
  uint8_t type_id, xense_callback callback) {
return serialize_xense_message({&data, Xense_data_fields},
  buffer, buffer_size, type_id, callback);
}

/**
 * Deserialize the Xense data structure from a buffer.
 *
 * @param buffer The buffer to deserialize the data from.
 * @param buffer_size The size of the buffer.
 * @param data The Xense data structure to deserialize into.
 * @param callback The callback to call after serialization.
 * @return True if the deserialization was successful, false otherwise.
 */
XenseStatus deserialize_xense_data(const uint8_t *buffer, size_t buffer_size,
  Xense_data &data,
  xense_callback callback) {
return deserialize_xense_message({&data, Xense_data_fields},
  buffer, buffer_size, callback);
}