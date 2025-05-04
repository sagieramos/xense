#include "xense.h"

/**
 * @brief Serialize the Xense metadata structure into a buffer.
 *
 * @param metadata The Xense metadata structure to serialize.
 * @param buffer The buffer to serialize the metadata into.
 * @param buffer_size The size of the buffer.
 * @param type_id The type ID to prepend to the buffer.
 * @param callback The callback to call after serialization.
 *
 * @return XENSE_OK if the serialization was successful, an error code
 * otherwise.
 */

NpbStatus serialize_xense_metadata(const Xense_metadata &metadata,
                                   uint8_t *buffer, size_t *buffer_size,
                                   uint8_t type_id, uint8_t *mac_address,
                                   xense_callback callback) {
  return npb_encode({&metadata, Xense_metadata_fields}, buffer, buffer_size,
                    type_id, mac_address, callback);
}

/**
 * @brief Deserialize the Xense metadata structure from a buffer.
 *
 * @param buffer The buffer to deserialize the metadata from.
 * @param buffer_size The size of the buffer.
 * @param metadata The Xense metadata structure to deserialize into.
 * @param callback The callback to call after serialization.
 *
 * @return True if the deserialization was successful, false otherwise.
 */

NpbStatus deserialize_xense_metadata(const uint8_t *buffer, size_t buffer_size,
                                     Xense_metadata &metadata,
                                     xense_callback callback) {
  return npb_decode({&metadata, Xense_metadata_fields}, buffer, buffer_size,
                    callback);
}