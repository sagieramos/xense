#include "xense.h"

XenseStatus serialize_xense_message(const XenseMessageDescriptor &desc,
    uint8_t *buffer, size_t *buffer_size,
    uint8_t type_id,
    xense_callback callback) {
if (!buffer || !buffer_size || !desc.serialized_message || !desc.fields)
return XENSE_NULL_BUFFER;

buffer[0] = type_id;
pb_ostream_t stream = pb_ostream_from_buffer(buffer + 1, *buffer_size - 1);
if (!pb_encode(&stream, desc.fields, desc.serialized_message))
return XENSE_ENCODE_FAILED;

*buffer_size = stream.bytes_written + 1;
if (callback) callback(buffer, *buffer_size);
return XENSE_OK;
}

XenseStatus deserialize_xense_message(const XenseMessageDescriptor &desc,
      const uint8_t *buffer, size_t buffer_size,
      xense_callback callback) {
if (!buffer || buffer_size < 2 || !desc.deserialized_message || !desc.fields)
return XENSE_INVALID_SIZE;

pb_istream_t stream = pb_istream_from_buffer(buffer + 1, buffer_size - 1);
if (!pb_decode(&stream, desc.fields, desc.deserialized_message))
return XENSE_DECODE_FAILED;

if (callback) callback(buffer, buffer_size);
return XENSE_OK;
}
