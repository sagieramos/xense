#ifndef NPB_TRANSCODE_H
#define NPB_TRANSCODE_H

#include "../../external/nanopb/pb.h"
#include "../../external/nanopb/pb_common.h"
#include "../../external/nanopb/pb_decode.h"
#include "../../external/nanopb/pb_encode.h"

typedef enum {
  NPB_OK,
  NPB_NULL_BUFFER,
  NPB_ENCODE_FAILED,
  NPB_DECODE_FAILED,
  NPB_INVALID_SIZE
} NpbStatus;

typedef void (*xense_callback)(const void *buffer, size_t size);

struct NpbMessageDescriptor {
  const void *serialized_message = nullptr; // For serialization
  void *deserialized_message = nullptr;     // For deserialization
  const pb_msgdesc_t *fields = nullptr;     // Message descriptor

  NpbMessageDescriptor(const void *msg, const pb_msgdesc_t *fld)
      : serialized_message(msg), fields(fld) {}

  NpbMessageDescriptor(void *mutable_msg, const pb_msgdesc_t *fld)
      : deserialized_message(mutable_msg), fields(fld) {}
};

// Generic functions
NpbStatus npb_encode(const NpbMessageDescriptor &desc, uint8_t *buffer,
                     size_t *buffer_size, uint8_t type_id, uint8_t *mac_address,
                     xense_callback callback);

NpbStatus npb_decode(const NpbMessageDescriptor &desc, const uint8_t *buffer,
                     size_t buffer_size, xense_callback callback = nullptr);

NpbStatus get_mac_address_from_npb(const uint8_t *buffer, size_t buffer_size,
                                   uint8_t *mac_address);

#endif // NPB_TRANSCODE_H