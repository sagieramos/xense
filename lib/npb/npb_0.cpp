#include "./npb_transcode.h"

#define TYPE_ID 1
#define MAC_ADDRESS 6

NpbStatus npb_encode(const NpbMessageDescriptor &desc, uint8_t *buffer,
                     size_t *buffer_size, uint8_t type_id, uint8_t *mac_address,
                     xense_callback callback) {
  if (!buffer || !buffer_size || !desc.serialized_message || !desc.fields ||
      !mac_address)
    return NPB_NULL_BUFFER;

  if (*buffer_size < TYPE_ID + MAC_ADDRESS)
    return NPB_INVALID_SIZE;

  buffer[0] = type_id;
  memcpy(buffer + TYPE_ID, mac_address, MAC_ADDRESS);

  pb_ostream_t stream = pb_ostream_from_buffer(
      buffer + TYPE_ID + MAC_ADDRESS, *buffer_size - TYPE_ID - MAC_ADDRESS);
  if (!pb_encode(&stream, desc.fields, desc.serialized_message))
    return NPB_ENCODE_FAILED;

  size_t message_size = TYPE_ID + MAC_ADDRESS + stream.bytes_written;

  *buffer_size = message_size;

  if (callback)
    callback(buffer, *buffer_size);

  return NPB_OK;
}

NpbStatus npb_decode(const NpbMessageDescriptor &desc, const uint8_t *buffer,
                     size_t buffer_size, xense_callback callback) {
  if (!buffer || buffer_size < TYPE_ID + MAC_ADDRESS ||
      !desc.deserialized_message || !desc.fields)
    return NPB_INVALID_SIZE;

  pb_istream_t stream = pb_istream_from_buffer(
      buffer + TYPE_ID + MAC_ADDRESS, buffer_size - TYPE_ID - MAC_ADDRESS);
  if (!pb_decode(&stream, desc.fields, desc.deserialized_message))
    return NPB_DECODE_FAILED;

  if (callback)
    callback(buffer, buffer_size);

  return NPB_OK;
}

NpbStatus get_mac_address_from_npb(const uint8_t *buffer, size_t buffer_size,
                                   uint8_t *mac_address) {
  if (!buffer || buffer_size < TYPE_ID + MAC_ADDRESS || !mac_address)
    return NPB_NULL_BUFFER;

  memcpy(mac_address, buffer + TYPE_ID, MAC_ADDRESS);
  return NPB_OK;
}
