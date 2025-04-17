#ifndef XENSE_H
#define XENSE_H

#include "../../external/nanopb/pb.h"
#include "../../external/nanopb/pb_common.h"
#include "../../external/nanopb/pb_decode.h"
#include "../../external/nanopb/pb_encode.h"
#include "./protoc/xense.pb.h"

/* Type Identifiers (for use in manual encoding/decoding) */
#define XENSE_GPS_TYPE_ID 2
#define XENSE_METADATA_TYPE_ID 3
#define XENSE_STATE_TYPE_ID 4
#define XENSE_DATA_TYPE_ID 5

// xense_common.h
typedef enum
{
  XENSE_OK,
  XENSE_NULL_BUFFER,
  XENSE_ENCODE_FAILED,
  XENSE_DECODE_FAILED,
  XENSE_INVALID_SIZE
} XenseStatus;

typedef void (*xense_callback)(const void *buffer, size_t size);

struct XenseMessageDescriptor
{
  const void *serialized_message = nullptr; // For serialization
  void *deserialized_message = nullptr;     // For deserialization
  const pb_msgdesc_t *fields = nullptr;     // Message descriptor

  XenseMessageDescriptor(const void *msg, const pb_msgdesc_t *fld)
      : serialized_message(msg), fields(fld) {}

  XenseMessageDescriptor(void *mutable_msg, const pb_msgdesc_t *fld)
      : deserialized_message(mutable_msg), fields(fld) {}
};

// Generic functions
XenseStatus serialize_xense_message(const XenseMessageDescriptor &desc,
                                    uint8_t *buffer, size_t *buffer_size,
                                    uint8_t type_id,
                                    xense_callback callback = nullptr);

XenseStatus deserialize_xense_message(const XenseMessageDescriptor &desc,
                                      const uint8_t *buffer, size_t buffer_size,
                                      xense_callback callback = nullptr);

XenseStatus serialize_xense_data(const Xense_data &data,
                                 uint8_t *buffer, size_t *buffer_size,
                                 uint8_t type_id,
                                 xense_callback callback = nullptr);

XenseStatus deserialize_xense_data(const uint8_t *buffer, size_t buffer_size,
                                   Xense_data &data,
                                   xense_callback callback = nullptr);

XenseStatus serialize_xense_gps(const Xense_gps &coordinate,
                                        uint8_t *buffer, size_t *buffer_size,
                                        uint8_t type_id,
                                        xense_callback callback = nullptr);
XenseStatus deserialize_xense_gps(const uint8_t *buffer, size_t buffer_size,
                                          Xense_gps &coordinate,
                                          xense_callback callback = nullptr);

#endif // XENSE_H